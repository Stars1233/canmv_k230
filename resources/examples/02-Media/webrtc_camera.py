# WebRTC camera example for CanMV K230.
# Open http://<board-ip>:8080 in a browser after the network is connected.

from media.vencoder import *
from media.sensor import *
from media.media import *
import _thread
import network
import os
import socket
import sys
import time
import uctypes
import webrtc

NETWORK_MODE = "lan"  # "lan", "wifi_sta", or "wifi_ap"
WIFI_SSID = "Test"
WIFI_PASSWORD = "12345678"
HTTP_PORT = 8080
WIDTH = 1280
HEIGHT = 720
VIDEO_CODEC = "h265"  # "h265" or "h264"
BIT_RATE = 512  # Kbit/s
AUDIO_CODEC = webrtc.CODEC_NONE

DEVICE_NAMES = {
    "lan": "u0",
    "wifi_sta": "w0",
    "wifi_ap": "w1",
}

PAGE = b"""<!doctype html>
<html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>CanMV WebRTC</title><style>
*{box-sizing:border-box}body{margin:0;background:#111;color:#eee;font:14px sans-serif;height:100vh;display:flex;flex-direction:column}
header{height:52px;flex:0 0 52px;display:flex;align-items:center;gap:12px;padding:0 14px;background:#242424;border-bottom:1px solid #383838}
header strong{margin-right:auto;white-space:nowrap}button{min-width:88px;height:34px;border:0;border-radius:5px;padding:0 14px;background:#1683ff;color:white;cursor:pointer}
button:disabled{cursor:wait;opacity:.6}#state{color:#aaa;white-space:nowrap}.viewer{position:relative;flex:1;min-height:0;background:#000}
video{display:block;width:100%;height:100%;object-fit:contain}.stage{position:absolute;inset:0;display:flex;flex-direction:column;align-items:center;justify-content:center;padding:24px;background:rgba(0,0,0,.72);opacity:0;visibility:hidden;transition:opacity .18s}
.stage.show{opacity:1;visibility:visible}.viewer.playing .stage{opacity:0;visibility:hidden}.spinner{width:30px;height:30px;margin-bottom:16px;border:3px solid #555;border-top-color:#2f9bff;border-radius:50%;animation:spin .8s linear infinite}
.stage.error .spinner{display:none}.stage-title{font-size:18px;line-height:24px}.stage-detail{max-width:430px;margin-top:7px;color:#aaa;line-height:20px;text-align:center}
@keyframes spin{to{transform:rotate(360deg)}}@media(max-width:480px){header{gap:8px;padding:0 10px}header strong{font-size:13px}button{min-width:76px;padding:0 10px}#state{font-size:12px}}
</style></head><body><header><strong>CanMV WebRTC</strong><span id="state" aria-live="polite">Ready</span><button id="connect">Connect</button></header>
<main class="viewer"><video id="video" autoplay playsinline muted></video><div id="stage" class="stage" role="status" aria-live="polite"><div class="spinner"></div><div id="stage-title" class="stage-title"></div><div id="stage-detail" class="stage-detail"></div></div></main><script>
let pc=null,attempt=0,streamingAttempt=0,elapsedTimer=null,retryTimer=null,startedAt=0;
const state=document.querySelector('#state'),button=document.querySelector('#connect'),video=document.querySelector('#video'),viewer=document.querySelector('.viewer'),stage=document.querySelector('#stage'),stageTitle=document.querySelector('#stage-title'),stageDetail=document.querySelector('#stage-detail');
function waitCandidate(p){return new Promise(resolve=>{let timer;
const done=()=>{p.removeEventListener('icecandidate',done);clearTimeout(timer);resolve()};
p.addEventListener('icecandidate',done);timer=setTimeout(done,3000)})}
function clearTimers(){clearInterval(elapsedTimer);clearTimeout(retryTimer);elapsedTimer=null;retryTimer=null}
function showStep(title,detail,id){if(id!==undefined&&(id!==attempt||streamingAttempt===id||viewer.classList.contains('playing')))return;stage.className='stage show';stageTitle.textContent=title;stageDetail.textContent=detail}
function beginWait(){clearTimers();startedAt=Date.now();button.disabled=true;button.textContent='Connecting';state.textContent='Connecting 0s';showStep('Preparing camera','Requesting the encoded video stream from the board.');
elapsedTimer=setInterval(()=>{state.textContent='Connecting '+Math.floor((Date.now()-startedAt)/1000)+'s'},1000);
retryTimer=setTimeout(()=>{button.disabled=false;button.textContent='Retry';stageDetail.textContent='Still connecting. Network negotiation can take longer on a busy Wi-Fi link.'},15000)}
function connected(id){if(id!==attempt)return;streamingAttempt=id;clearTimers();state.textContent='Connected';button.disabled=false;button.textContent='Reconnect';stage.className='stage'}
function failed(id,detail){if(id!==attempt)return;clearTimers();state.textContent='Failed';button.disabled=false;button.textContent='Retry';stage.className='stage show error';stageTitle.textContent='Connection failed';stageDetail.textContent=detail}
function closePeer(){if(!pc)return;pc.ontrack=null;pc.oniceconnectionstatechange=null;pc.onconnectionstatechange=null;pc.close();pc=null}
video.addEventListener('playing',()=>{viewer.classList.add('playing');connected(attempt)});
button.onclick=async()=>{const id=++attempt;streamingAttempt=0;closePeer();video.srcObject=null;viewer.classList.remove('playing');beginWait();pc=new RTCPeerConnection({iceServers:[]});
pc.ontrack=e=>{if(id!==attempt)return;video.srcObject=e.streams[0];showStep('Starting video','The video track is ready. Waiting for the first frame to play.',id)};
pc.oniceconnectionstatechange=()=>{if(id!==attempt)return;const s=pc.iceConnectionState;if(streamingAttempt===id&&(s==='checking'||s==='connected'||s==='completed'))return;
if(s==='checking')showStep('Connecting secure stream','Checking the local network path to the camera.',id);
else if(s==='connected'||s==='completed')showStep('Starting video','The secure link is ready. Waiting for the first encoded frame.',id);
else if(s==='disconnected')showStep('Reconnecting','The network path was interrupted. Waiting for it to recover.',id);
else if(s==='failed')failed(id,'The browser could not reach the camera. Check the network and try again.')};
pc.onconnectionstatechange=()=>{if(id===attempt&&pc.connectionState==='failed')failed(id,'The secure connection failed. Try H.264 if this browser does not support H.265.')};
try{showStep('Preparing camera','Requesting a WebRTC offer from the board.',id);let response=await fetch('/offer');if(!response.ok)throw new Error();let offer=await response.text();if(id!==attempt)return;
showStep('Negotiating video','Selecting a compatible video codec.',id);await pc.setRemoteDescription({type:'offer',sdp:offer});
let answer=await pc.createAnswer(),candidate=waitCandidate(pc);showStep('Finding network path','Gathering the browser network address.',id);await pc.setLocalDescription(answer);await candidate;if(id!==attempt)return;
showStep('Connecting secure stream','Sending the browser response to the camera.',id);response=await fetch('/answer',{method:'POST',headers:{'Content-Type':'application/sdp'},body:pc.localDescription.sdp});if(!response.ok)throw new Error();
showStep('Starting video','Connection details are ready. Waiting for the first encoded frame.',id)}catch(e){if(streamingAttempt!==id)failed(id,'Unable to complete negotiation. Try H.264 if this browser does not support H.265.')}};</script></body></html>"""


def wait_for_ip(nic, require_connection=False, timeout=15):
    start = time.time()
    while time.time() - start < timeout:
        config = nic.ifconfig()
        if (not require_connection or nic.isconnected()) and config and config[0] != "0.0.0.0":
            return config[0]
        time.sleep_ms(200)
    raise RuntimeError("network did not obtain an IP address")


def connect_network():
    if NETWORK_MODE not in DEVICE_NAMES:
        raise ValueError("invalid NETWORK_MODE")
    devices = network.get_dev_list()
    device = DEVICE_NAMES[NETWORK_MODE]
    if devices is None or device not in devices:
        raise RuntimeError("network device '%s' is not available" % device)

    if NETWORK_MODE == "lan":
        nic = network.LAN()
        if nic.ifconfig("dhcp") is False:
            raise RuntimeError("LAN DHCP failed")
        ip = wait_for_ip(nic, True)
    elif NETWORK_MODE == "wifi_sta":
        nic = network.WLAN(network.STA_IF)
        nic.active(True)
        if not nic.isconnected() and nic.connect(WIFI_SSID, WIFI_PASSWORD) is False:
            raise RuntimeError("Wi-Fi connection failed")
        ip = wait_for_ip(nic, True)
    else:
        nic = network.WLAN(network.AP_IF)
        if nic.config(ssid=WIFI_SSID, key=WIFI_PASSWORD) is False:
            raise RuntimeError("Wi-Fi AP start failed")
        ip = wait_for_ip(nic)

    if network.set_default_dev(device) is False:
        raise RuntimeError("failed to select the default network device")
    return nic, ip


def send_all(sock, data):
    offset = 0
    while offset < len(data):
        sent = sock.send(data[offset:])
        if sent <= 0:
            raise OSError("socket send failed")
        offset += sent


def send_response(client, status, content_type, body):
    header = ("HTTP/1.1 %s\r\nContent-Type: %s\r\nContent-Length: %d\r\n"
              "Cache-Control: no-store, no-cache, must-revalidate\r\nPragma: no-cache\r\nExpires: 0\r\n"
              "Access-Control-Allow-Origin: *\r\nAccess-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
              "Access-Control-Allow-Headers: Content-Type\r\nConnection: close\r\n\r\n" %
              (status, content_type, len(body))).encode()
    send_all(client, header)
    send_all(client, body)


def read_request(client):
    data = b""
    while data.find(b"\r\n\r\n") < 0 and len(data) < 16384:
        chunk = client.recv(2048)
        if not chunk:
            break
        data += chunk
    split = data.find(b"\r\n\r\n")
    if split < 0:
        return None
    try:
        header = data[:split].decode()
    except UnicodeError:
        return None
    body = data[split + 4:]
    lines = header.split("\r\n")
    request_line = lines[0].split(" ", 2)
    if len(request_line) != 3:
        return None
    method, path, _ = request_line
    content_length = 0
    for line in lines[1:]:
        if line.lower().startswith("content-length:"):
            try:
                content_length = int(line.split(":", 1)[1].strip())
            except ValueError:
                return None
            break
    while len(body) < content_length:
        chunk = client.recv(min(2048, content_length - len(body)))
        if not chunk:
            break
        body += chunk
    return method, path, body[:content_length]


def replace_mdns_candidates(sdp, client_ip):
    lines = sdp.split("\r\n")
    for index in range(len(lines)):
        if not lines[index].startswith("a=candidate:"):
            continue
        fields = lines[index].split(" ")
        if len(fields) > 4 and fields[4].endswith(".local"):
            fields[4] = client_ip
            lines[index] = " ".join(fields)
    return "\r\n".join(lines)


def signaling_server(peer, stop):
    server = socket.socket()
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(("0.0.0.0", HTTP_PORT))
    server.listen(2)
    server.settimeout(0.5)
    try:
        while not stop[0]:
            try:
                client, client_address = server.accept()
            except OSError:
                continue
            try:
                client.settimeout(5)
                request = read_request(client)
                if request is None:
                    continue
                method, path, body = request
                if method == "OPTIONS":
                    send_response(client, "204 No Content", "text/plain", b"")
                elif method == "GET" and path == "/":
                    send_response(client, "200 OK", "text/html", PAGE)
                elif method == "GET" and path == "/offer":
                    send_response(client, "200 OK", "application/sdp", peer.create_offer().encode())
                elif method == "POST" and path == "/answer":
                    answer = replace_mdns_candidates(body.decode(), client_address[0])
                    peer.set_remote_description(answer)
                    send_response(client, "200 OK", "text/plain", b"OK")
                else:
                    send_response(client, "404 Not Found", "text/plain", b"Not Found")
            except OSError as error:
                if not error.args or error.args[0] not in (32, 104):
                    sys.print_exception(error)
            except BaseException as error:
                sys.print_exception(error)
            finally:
                client.close()
    finally:
        server.close()


def run():
    width = ALIGN_UP(WIDTH, 16)
    stop = [False]
    sensor = None
    encoder = None
    link = None
    peer = None

    _, ip = connect_network()
    try:
        if VIDEO_CODEC == "h265":
            peer_video_codec = webrtc.CODEC_H265
            payload_type = Encoder.PAYLOAD_TYPE_H265
            profile = Encoder.H265_PROFILE_MAIN
        elif VIDEO_CODEC == "h264":
            peer_video_codec = webrtc.CODEC_H264
            payload_type = Encoder.PAYLOAD_TYPE_H264
            profile = Encoder.H264_PROFILE_MAIN
        else:
            raise ValueError("VIDEO_CODEC must be 'h265' or 'h264'")

        peer = webrtc.PeerConnection(video_codec=peer_video_codec,
                                     audio_codec=AUDIO_CODEC)
        sensor = Sensor()
        sensor.reset()
        sensor.set_framesize(width=width, height=HEIGHT, alignment=12)
        sensor.set_pixformat(Sensor.YUV420SP)

        encoder = Encoder()
        encoder.SetOutBufs(8, width, HEIGHT)
        attributes = ChnAttrStr(payload_type, profile, width, HEIGHT,
                                bit_rate=BIT_RATE)
        encoder.Create(attributes)
        link = MediaManager.link(sensor.bind_info()["src"],
                                 (VIDEO_ENCODE_MOD_ID, VENC_DEV_ID, encoder.chn))
        encoder.Start()
        sensor.run()
        _thread.start_new_thread(signaling_server, (peer, stop))
        print("WebRTC camera: http://%s:%d (%s, %d Kbit/s, audio disabled)" %
              (ip, HTTP_PORT, VIDEO_CODEC.upper(), BIT_RATE))

        stream = StreamData()
        parameter_sets = None
        was_connected = False
        while True:
            os.exitpoint()
            connected = peer.is_connected()
            if connected and not was_connected:
                # Do not wait for the next periodic keyframe after ICE/DTLS completes.
                encoder.RequestIDR()
            was_connected = connected
            if encoder.GetStream(stream, timeout=100) != 0:
                continue
            try:
                for index in range(stream.pack_cnt):
                    data = uctypes.bytearray_at(stream.data[index], stream.data_size[index])
                    stream_type = stream.stream_type[index]
                    timestamp = stream.pts[index]
                    if stream_type == encoder.STREAM_TYPE_HEADER:
                        parameter_sets = bytes(data)
                    elif connected:
                        if stream_type == encoder.STREAM_TYPE_I and parameter_sets:
                            peer.send_video(parameter_sets, timestamp)
                        peer.send_video(data, timestamp)
            finally:
                encoder.ReleaseStream(stream)
    except KeyboardInterrupt:
        pass
    finally:
        stop[0] = True
        if sensor is not None:
            sensor.stop()
        if link is not None:
            link.destroy()
        if encoder is not None and encoder.chn >= 0:
            encoder.Stop()
            encoder.Destroy()
        if peer is not None:
            peer.close()


if __name__ == "__main__":
    os.exitpoint(os.EXITPOINT_ENABLE)
    run()
