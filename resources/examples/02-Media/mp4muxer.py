# Save MP4 file example
#
# Note: You will need an SD card to run this example.
#
# You can capture audio and video and save them as MP4.The current version only supports MP4 format, video supports 264/265, and audio supports g711a/g711u.
from media.mp4format import *
from mpp.mp4_format import *
from mpp.mp4_format_struct import *
from media.vencoder import *
from media.sensor import *
from media.media import *
import uctypes
import time
import os

def mp4_muxer_init(file_name,  fmp4_flag):
    mp4_cfg = k_mp4_config_s()
    mp4_cfg.config_type = K_MP4_CONFIG_MUXER
    mp4_cfg.muxer_config.file_name[:] = bytes(file_name, 'utf-8')
    mp4_cfg.muxer_config.fmp4_flag = fmp4_flag

    handle = k_u64_ptr()
    ret = kd_mp4_create(handle, mp4_cfg)
    if ret:
        raise OSError("kd_mp4_create failed.")
    return handle.value

def mp4_muxer_create_video_track(mp4_handle, width, height, video_payload_type):
    video_track_info = k_mp4_track_info_s()
    video_track_info.track_type = K_MP4_STREAM_VIDEO
    video_track_info.time_scale = 1000
    video_track_info.video_info.width = width
    video_track_info.video_info.height = height
    video_track_info.video_info.codec_id = video_payload_type
    video_track_handle = k_u64_ptr()
    ret = kd_mp4_create_track(mp4_handle, video_track_handle, video_track_info)
    if ret:
        raise OSError("kd_mp4_create_track failed.")
    return video_track_handle.value

def mp4_muxer_create_audio_track(mp4_handle,channel,sample_rate, bit_per_sample ,audio_payload_type):
    audio_track_info = k_mp4_track_info_s()
    audio_track_info.track_type = K_MP4_STREAM_AUDIO
    audio_track_info.time_scale = 1000
    audio_track_info.audio_info.channels = channel
    audio_track_info.audio_info.codec_id = audio_payload_type
    audio_track_info.audio_info.sample_rate = sample_rate
    audio_track_info.audio_info.bit_per_sample = bit_per_sample
    audio_track_handle = k_u64_ptr()
    ret = kd_mp4_create_track(mp4_handle, audio_track_handle, audio_track_info)
    if ret:
        raise OSError("kd_mp4_create_track failed.")
    return audio_track_handle.value

def vi_bind_venc_mp4_test(file_name,width=1280, height=720,venc_payload_type = K_PT_H264,fmp4_flag = False):
    print("venc_test start")
    venc_chn = VENC_CHN_ID_0
    width = ALIGN_UP(width, 16)

    frame_data = k_mp4_frame_data_s()
    save_idr = bytearray(width * height * 3 // 4)
    idr_index = 0

    # mp4 muxer init
    mp4_handle = mp4_muxer_init(file_name, fmp4_flag)

    # create video track
    if venc_payload_type == K_PT_H264:
        video_payload_type = K_MP4_CODEC_ID_H264
    elif venc_payload_type == K_PT_H265:
        video_payload_type = K_MP4_CODEC_ID_H265
    mp4_video_track_handle = mp4_muxer_create_video_track(mp4_handle, width, height, video_payload_type)

    # 初始化sensor
    sensor = Sensor()
    sensor.reset()
    # 设置camera 输出buffer
    # set chn0 output size
    sensor.set_framesize(width = width, height = height, alignment=12)
    # set chn0 output format
    sensor.set_pixformat(Sensor.YUV420SP)

    # 实例化video encoder
    encoder = Encoder()
    # 设置video encoder 输出buffer
    encoder.SetOutBufs(venc_chn, 8, width, height)

    # 绑定camera和venc
    link = MediaManager.link(sensor.bind_info()['src'], (VIDEO_ENCODE_MOD_ID, VENC_DEV_ID, venc_chn))

    # init media manager
    MediaManager.init()

    if (venc_payload_type == K_PT_H264):
        chnAttr = ChnAttrStr(encoder.PAYLOAD_TYPE_H264, encoder.H264_PROFILE_MAIN, width, height)
    elif (venc_payload_type == K_PT_H265):
        chnAttr = ChnAttrStr(encoder.PAYLOAD_TYPE_H265, encoder.H265_PROFILE_MAIN, width, height)

    streamData = StreamData()

    # 创建编码器
    encoder.Create(venc_chn, chnAttr)

    # 开始编码
    encoder.Start(venc_chn)
    # 启动camera
    sensor.run()

    frame_count = 0
    print("save stream to file: ", file_name)

    video_start_timestamp = 0
    get_first_I_frame = False

    try:
        while True:
            os.exitpoint()
            encoder.GetStream(venc_chn, streamData) # 获取一帧码流
            stream_type = streamData.stream_type[0]

            # Retrieve first IDR frame and write to MP4 file. Note: The first frame must be an IDR frame.
            if not get_first_I_frame:
                if stream_type == encoder.STREAM_TYPE_I:
                    get_first_I_frame = True
                    video_start_timestamp = streamData.pts[0]
                    save_idr[idr_index:idr_index+streamData.data_size[0]] = uctypes.bytearray_at(streamData.data[0], streamData.data_size[0])
                    idr_index += streamData.data_size[0]

                    frame_data.codec_id = video_payload_type
                    frame_data.data = uctypes.addressof(save_idr)
                    frame_data.data_length = idr_index
                    frame_data.time_stamp = streamData.pts[0] - video_start_timestamp

                    ret = kd_mp4_write_frame(mp4_handle, mp4_video_track_handle, frame_data)
                    if ret:
                        raise OSError("kd_mp4_write_frame failed.")
                    encoder.ReleaseStream(venc_chn, streamData)
                    continue

                elif stream_type == encoder.STREAM_TYPE_HEADER:
                    save_idr[idr_index:idr_index+streamData.data_size[0]] = uctypes.bytearray_at(streamData.data[0], streamData.data_size[0])
                    idr_index += streamData.data_size[0]
                    encoder.ReleaseStream(venc_chn, streamData)
                    continue
                else:
                    encoder.ReleaseStream(venc_chn, streamData) # 释放一帧码流
                    continue

            # Write video stream to MP4 file （not first idr frame）
            frame_data.codec_id = video_payload_type
            frame_data.data = streamData.data[0]
            frame_data.data_length = streamData.data_size[0]
            frame_data.time_stamp = streamData.pts[0] - video_start_timestamp

            #print("video size: ", streamData.data_size[0], "video type: ", streamData.stream_type[0],"video timestamp:",frame_data.time_stamp)
            ret = kd_mp4_write_frame(mp4_handle, mp4_video_track_handle, frame_data)
            if ret:
                raise OSError("kd_mp4_write_frame failed.")

            encoder.ReleaseStream(venc_chn, streamData) # 释放一帧码流

            frame_count += 1
            print("frame_count = ", frame_count)
            if frame_count >= 200:
                break
    except KeyboardInterrupt as e:
        print("user stop: ", e)
    except BaseException as e:
        import sys
        sys.print_exception(e)

    # 停止camera
    sensor.stop()
    # 销毁camera和venc的绑定
    del link
    # 停止编码
    encoder.Stop(venc_chn)
    # 销毁编码器
    encoder.Destroy(venc_chn)
    # 清理buffer
    MediaManager.deinit()

    # mp4 muxer destroy
    kd_mp4_destroy_tracks(mp4_handle)
    kd_mp4_destroy(mp4_handle)

    print("record mp4 done")

def mp4_muxer_test():
    print("mp4_muxer_test start")
    width = 1280
    height = 720
    # 实例化mp4 container
    mp4_muxer = Mp4Container()
    #mp4_muxer = Mp4Container(sensor_csi_id = 2)  # 指定探测CSI2的sensor
    mp4_cfg = Mp4CfgStr(mp4_muxer.MP4_CONFIG_TYPE_MUXER)
    if mp4_cfg.type == mp4_muxer.MP4_CONFIG_TYPE_MUXER:
        file_name = "/sdcard/examples/test.mp4"
        mp4_cfg.SetMuxerCfg(file_name, mp4_muxer.MP4_CODEC_ID_H265, width, height, mp4_muxer.MP4_CODEC_ID_G711U)
    # 创建mp4 muxer
    mp4_muxer.Create(mp4_cfg)
    # 启动mp4 muxer
    mp4_muxer.Start()

    frame_count = 0
    try:
        while True:
            os.exitpoint()
            # 处理音视频数据，按MP4格式写入文件
            mp4_muxer.Process()
            frame_count += 1
            print("frame_count = ", frame_count)
            if frame_count >= 200:
                break
    except BaseException as e:
        print(e)
    # 停止mp4 muxer
    mp4_muxer.Stop()
    # 销毁mp4 muxer
    mp4_muxer.Destroy()
    print("mp4_muxer_test stop")

if __name__ == "__main__":
    os.exitpoint(os.EXITPOINT_ENABLE)
    vi_bind_venc_mp4_test("/sdcard/examples/test.mp4", 1280, 720)
    #mp4_muxer_test()
