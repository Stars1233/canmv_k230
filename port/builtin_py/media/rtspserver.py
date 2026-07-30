from media.vencoder import *
from media.sensor import *
from media.media import *
import time, os
import _thread
import multimedia as mm
from time import *

class RtspServer:
    def __init__(self, session_name="test", port=8554,
                 video_type=mm.multi_media_type.media_h265, enable_audio=False,
                 width=1280, height=720, bit_rate=512, gop_len=30):
        """Initialize the object.
        Args:
            session_name: RTSP session name.
            port: RTSP listening port.
            video_type: RTSP video encoding type.
            enable_audio: Whether to enable audio streaming.
            width: Encoded video width.
            height: Encoded video height.
            bit_rate: Target video bit rate in Kbit/s.
            gop_len: Number of frames per GOP.
        """
        self.session_name = session_name
        self.video_type = video_type
        self.enable_audio = enable_audio
        self.port = port
        self.width = ALIGN_UP(width, 16)
        self.height = height
        self.bit_rate = bit_rate
        self.gop_len = gop_len
        self.rtspserver = mm.rtsp_server()
        self.start_stream = False
        self.runthread_over = False

        if bit_rate < 100 or bit_rate > 20000:
            raise ValueError("bit_rate must be between 100 and 20000 Kbit/s")
        if video_type == mm.multi_media_type.media_h265:
            self.payload_type = Encoder.PAYLOAD_TYPE_H265
            self.profile = Encoder.H265_PROFILE_MAIN
        elif video_type == mm.multi_media_type.media_h264:
            self.payload_type = Encoder.PAYLOAD_TYPE_H264
            self.profile = Encoder.H264_PROFILE_MAIN
        else:
            raise ValueError("video_type must be media_h265 or media_h264")

    def start(self):
        """Start processing.
        """
        self._init_stream()
        self.rtspserver.rtspserver_init(self.port)
        self.rtspserver.rtspserver_createsession(self.session_name,self.video_type,self.enable_audio)
        self.rtspserver.rtspserver_start()
        self._start_stream()

        self.start_stream = True
        _thread.start_new_thread(self._do_rtsp_stream,())


    def stop(self):
        """Stop processing.
        """
        self.start_stream = False
        while not self.runthread_over:
            sleep(0.1)
        self.runthread_over = False

        self._stop_stream()
        self.rtspserver.rtspserver_stop()
        #self.rtspserver.rtspserver_destroysession(self.session_name)
        self.rtspserver.rtspserver_deinit()

    def get_rtsp_url(self):
        """Return the RTSP server URL.
        """
        return self.rtspserver.rtspserver_getrtspurl(self.session_name)

    def _init_stream(self):
        """Internal helper method.
        """
        self.sensor = Sensor()
        self.sensor.reset()
        self.sensor.set_framesize(width=self.width, height=self.height, alignment=12)
        self.sensor.set_pixformat(Sensor.YUV420SP)
        self.encoder = Encoder()
        self.encoder.SetOutBufs(15, self.width, self.height)
        chnAttr = ChnAttrStr(self.payload_type, self.profile,
                             self.width, self.height,
                             bit_rate=self.bit_rate, gopLen=self.gop_len)
        self.encoder.Create(chnAttr)
        self.link = MediaManager.link(self.sensor.bind_info()['src'], (VIDEO_ENCODE_MOD_ID, VENC_DEV_ID, self.encoder.chn))

    def _start_stream(self):
        """Internal helper method.
        """
        self.encoder.Start()
        self.sensor.run()

    def _stop_stream(self):
        """Internal helper method.
        """
        self.sensor.stop()
        self.link.destroy()
        self.encoder.Stop()
        self.encoder.Destroy()

    def _do_rtsp_stream(self):
        """Internal helper method.
        """
        try:
            streamData = StreamData()
            while self.start_stream:
                os.exitpoint()
                self.encoder.GetStream(streamData) # 获取一帧码流

                for pack_idx in range(0, streamData.pack_cnt):
                    stream_data = bytes(uctypes.bytearray_at(streamData.data[pack_idx], streamData.data_size[pack_idx]))
                    self.rtspserver.rtspserver_sendvideodata(self.session_name,stream_data, streamData.data_size[pack_idx],1000)
                    #print("stream size: ", streamData.data_size[pack_idx], "stream type: ", streamData.stream_type[pack_idx])

                self.encoder.ReleaseStream(streamData) # 释放一帧码流

        except KeyboardInterrupt as e:
            print("user stop: ", e)
        except BaseException as e:
            sys.print_exception(e)

        self.runthread_over = True
        print("_do_rtsp_stream over")
