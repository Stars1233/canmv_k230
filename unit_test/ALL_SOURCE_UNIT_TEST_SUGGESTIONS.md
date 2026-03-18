# CANMV Source-Wide Unit Test Suggestions

Auto-generated on 2026-03-10 23:07:49 UTC.

| Source File | Suggested Unit Test Focus | Priority |
| --- | --- | --- |
| `fuzz_test/harnesses/fuzz_array.cpp` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `fuzz_test/harnesses/fuzz_ff_wrapper.cpp` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `fuzz_test/harnesses/fuzz_recent_fixes_guardrails.cpp` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `fuzz_test/harnesses/fuzz_ringbuf.cpp` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `fuzz_test/harnesses/fuzz_unaligned_memcpy.cpp` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `fuzz_test/src/micropython_host_stubs.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `fuzz_test/src/xalloc_host.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/lv_bindings/gen_mpy.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/3rd-party/lv_bindings/src/core/lv_obj.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/lv_bindings/src/core/lv_refr.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/lv_bindings/src/libs/freetype/lv_freetype.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/lv_bindings/src/libs/png/lodepng.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/lv_bindings/src/libs/png/lv_png.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/lv_bindings/src/misc/lv_color.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/lv_bindings/src/misc/lv_timer.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/lv_bindings/yb_config/lv_font_yb_cn_16.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/lv_bindings/yb_config/lv_font_yb_cn_22.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/ndarray.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/ndarray_operators.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/ndarray_properties.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/numpy/approx.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/numpy/bitwise.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/numpy/carray/carray.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/numpy/carray/carray_tools.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/numpy/compare.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/numpy/create.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/numpy/fft/fft.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/numpy/fft/fft_tools.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/numpy/filter.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/numpy/io/io.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/numpy/linalg/linalg.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/numpy/linalg/linalg_tools.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/numpy/ndarray/ndarray_iter.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/numpy/numerical.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/numpy/numpy.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/numpy/poly.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/numpy/stats.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/numpy/transform.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/numpy/vector.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/scipy/linalg/linalg.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/scipy/optimize/optimize.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/scipy/scipy.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/scipy/signal/signal.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/scipy/special/special.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/ulab.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/ulab_tools.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/user/user.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/3rd-party/ulab/code/utils/utils.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/ai_cube/ai_cube.c` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_cube/clipper.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_cube/fuction_ocrdet.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_cube/postprocess.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/ai_demo.c` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/anchors_640.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/body_seg.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/eye_gaze.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/face_detection.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/face_mesh.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/face_parse.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/kws/feature_pipeline.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/kws/fft.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/kws/kws_preprocess.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/licence_det.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/nanotracker.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/object_segment.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/ocr_rec.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/opencv_find_blobs.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/opencv_wrap.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/person_kp.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/rgb888_process.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/tts_zh/char_convert.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/tts_zh/chronology.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/tts_zh/finals.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/tts_zh/jieba_utils.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/tts_zh/num.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/tts_zh/phonecode.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/tts_zh/pinyin_utils.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/tts_zh/pypinyin.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/tts_zh/quantifier.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/tts_zh/standard.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/tts_zh/text_normalization.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/tts_zh/tone.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/tts_zh/tone_sanhi.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/tts_zh/tts_zh_preprocess.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/tts_zh/_utils.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/tts_zh/utils_tts.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/tts_zh/VoxCommon.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/tts_zh/zh_frontend.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/yolo_det.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/yolo_license_plate_det.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/yolo_obb.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/yolo_seg.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/ai_demo/yunet_postprocess.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/boards/k230_canmv_01studio/manifest.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/boards/k230_canmv_dongshanpi/manifest.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/boards/k230_canmv_gt6700/manifest.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/boards/k230_canmv_hiwonder/manifest.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/boards/k230_canmv_lckfb/manifest.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/boards/k230_canmv/manifest.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/boards/k230_canmv_rtt_evb/manifest.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/boards/k230_canmv_v3p0/manifest.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/boards/k230_canmv_wondermk/manifest.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/boards/k230_canmv_yahboom/manifest.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/boards/k230d_canmv_atk_dnk230d/manifest.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/boards/k230d_canmv_bpi_zero/manifest.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/boards/k230d_canmv_junroc_ai_cam/manifest.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/boards/k230d_canmv_mini/manifest.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/boards/k230d_evb/manifest.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/boards/k230d_labplus_ai_camera/manifest.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/boards/k230_evb/manifest.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/boards/k230_labplus_1956/manifest.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/boards/manifest.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/boards/modules/ds18x20_overlay/ds18x20.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/boards/modules/ds18x20_overlay/manifest.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/boards/modules/ds18x20_overlay/onewire.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/media/chunk.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/media/display.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/media/g711.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/media/__init__.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/media/media.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/media/mp4format.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/media/opus.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/media/player.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/media/pyaudio.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/media/rtspserver.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/media/sensor.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/media/uvc.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/media/vdecoder.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/media/vencoder.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/media/wave.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/adec_def.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/adec.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/adec_struct.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/aenc_def.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/aenc.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/aenc_struct.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/ai_def.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/ai.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/ai_struct.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/ao_def.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/ao.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/ao_struct.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/audio_def.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/audio_struct.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp_binding/adec_api.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/builtin_py/mpp_binding/aenc_api.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/builtin_py/mpp_binding/ai_api.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/builtin_py/mpp_binding/ao_api.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/builtin_py/mpp_binding/libogg_api.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/builtin_py/mpp_binding/modmpp.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/builtin_py/mpp_binding/mp4_format_api.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/builtin_py/mpp_binding/pm_api.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/builtin_py/mpp_binding/sys_api.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/builtin_py/mpp_binding/vb_api.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/builtin_py/mpp_binding/vdec_api.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/builtin_py/mpp_binding/venc_api.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/builtin_py/mpp_binding/vicap_api.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/builtin_py/mpp/common_struct.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/__init__.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/libogg_def.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/libogg.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/libogg_struct.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/mp4_format_def.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/mp4_format.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/mp4_format_struct.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/payload_struct.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/pm_def.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/pm.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/pm_struct.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/sensor_struct.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/sys_def.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/sys.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/sys_struct.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/vb_def.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/vb.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/vb_struct.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/vdec_def.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/vdec.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/vdec_struct.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/venc_def.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/venc.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/venc_struct.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/vicap_def.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/vicap.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/vicap_struct.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/video_def.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/builtin_py/mpp/video_struct.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `port/core/gccollect.c` | Boot/runtime state transitions, init/deinit order, and error-path tests with mocked RTOS/HAL. | High |
| `port/core/main.c` | Boot/runtime state transitions, init/deinit order, and error-path tests with mocked RTOS/HAL. | High |
| `port/core/mphalport.c` | Boot/runtime state transitions, init/deinit order, and error-path tests with mocked RTOS/HAL. | High |
| `port/core/mpp_vb_mgmt.c` | Boot/runtime state transitions, init/deinit order, and error-path tests with mocked RTOS/HAL. | High |
| `port/core/mpthreadport.c` | Boot/runtime state transitions, init/deinit order, and error-path tests with mocked RTOS/HAL. | High |
| `port/cv_lite/cv_lite.c` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/cv_lite/opencv_code/image_core.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/cv_lite/opencv_code/opencv_functions.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/kpu/ai2d.c` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/kpu/kpu.c` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/kpu/modnncase_runtime.c` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/kpu/nncase_wrap.cpp` | Inference wrapper contract tests with fake backend: tensor shape/type validation and deterministic output mapping. | Medium |
| `port/machine/machine_adc.c` | Driver API contract tests: argument validation, boundary values, and HAL-failure handling with mocks. | High |
| `port/machine/machine_bitstream.c` | Driver API contract tests: argument validation, boundary values, and HAL-failure handling with mocks. | High |
| `port/machine/machine_fft.c` | Driver API contract tests: argument validation, boundary values, and HAL-failure handling with mocks. | High |
| `port/machine/machine_fpioa.c` | Driver API contract tests: argument validation, boundary values, and HAL-failure handling with mocks. | High |
| `port/machine/machine_hw_spi.c` | Driver API contract tests: argument validation, boundary values, and HAL-failure handling with mocks. | High |
| `port/machine/machine_i2c.c` | Driver API contract tests: argument validation, boundary values, and HAL-failure handling with mocks. | High |
| `port/machine/machine_i2c_slave.c` | Driver API contract tests: argument validation, boundary values, and HAL-failure handling with mocks. | High |
| `port/machine/machine_led.c` | Driver API contract tests: argument validation, boundary values, and HAL-failure handling with mocks. | High |
| `port/machine/machine_pin.c` | Driver API contract tests: argument validation, boundary values, and HAL-failure handling with mocks. | High |
| `port/machine/machine_pwm.c` | Driver API contract tests: argument validation, boundary values, and HAL-failure handling with mocks. | High |
| `port/machine/machine_rotary_encoder.c` | Driver API contract tests: argument validation, boundary values, and HAL-failure handling with mocks. | High |
| `port/machine/machine_rtc.c` | Driver API contract tests: argument validation, boundary values, and HAL-failure handling with mocks. | High |
| `port/machine/machine_spi_lcd.c` | Driver API contract tests: argument validation, boundary values, and HAL-failure handling with mocks. | High |
| `port/machine/machine_timer.c` | Driver API contract tests: argument validation, boundary values, and HAL-failure handling with mocks. | High |
| `port/machine/machine_touch.c` | Driver API contract tests: argument validation, boundary values, and HAL-failure handling with mocks. | High |
| `port/machine/machine_uart.c` | Driver API contract tests: argument validation, boundary values, and HAL-failure handling with mocks. | High |
| `port/machine/machine_wdt.c` | Driver API contract tests: argument validation, boundary values, and HAL-failure handling with mocks. | High |
| `port/machine/modmachine.c` | Driver API contract tests: argument validation, boundary values, and HAL-failure handling with mocks. | High |
| `port/modules/modmedia.c` | Micropython module surface tests: import/init behavior, parameter checking, and return/error consistency. | Low |
| `port/modules/modmedia.display.c` | Micropython module surface tests: import/init behavior, parameter checking, and return/error consistency. | Low |
| `port/modules/modmedia.gsdma.c` | Micropython module surface tests: import/init behavior, parameter checking, and return/error consistency. | Low |
| `port/modules/modmedia.uvc.c` | Micropython module surface tests: import/init behavior, parameter checking, and return/error consistency. | Low |
| `port/modules/modmedia.videoframe.c` | Micropython module surface tests: import/init behavior, parameter checking, and return/error consistency. | Low |
| `port/modules/modnonai2d.c` | Micropython module surface tests: import/init behavior, parameter checking, and return/error consistency. | Low |
| `port/modules/modnonai2d.csc.c` | Micropython module surface tests: import/init behavior, parameter checking, and return/error consistency. | Low |
| `port/modules/modusb.c` | Micropython module surface tests: import/init behavior, parameter checking, and return/error consistency. | Low |
| `port/modules/modusb.serial.c` | Micropython module surface tests: import/init behavior, parameter checking, and return/error consistency. | Low |
| `port/modules/modvbmgmt.c` | Micropython module surface tests: import/init behavior, parameter checking, and return/error consistency. | Low |
| `port/mp_modules/modos.c` | Micropython module surface tests: import/init behavior, parameter checking, and return/error consistency. | Low |
| `port/mp_modules/modtermios.c` | Micropython module surface tests: import/init behavior, parameter checking, and return/error consistency. | Low |
| `port/mp_modules/modtime.c` | Micropython module surface tests: import/init behavior, parameter checking, and return/error consistency. | Low |
| `port/mp_modules/overlay_machine_pwm.c` | Micropython module surface tests: import/init behavior, parameter checking, and return/error consistency. | Low |
| `port/mp_modules/overlay_modcryptolib.c` | Micropython module surface tests: import/init behavior, parameter checking, and return/error consistency. | Low |
| `port/mp_modules/overlay_modgc.c` | Micropython module surface tests: import/init behavior, parameter checking, and return/error consistency. | Low |
| `port/mp_modules/overlay_modhashlib.c` | Micropython module surface tests: import/init behavior, parameter checking, and return/error consistency. | Low |
| `port/mp_modules/overlay_modonewire.c` | Micropython module surface tests: import/init behavior, parameter checking, and return/error consistency. | Low |
| `port/mp_modules/overlay_modos.c` | Micropython module surface tests: import/init behavior, parameter checking, and return/error consistency. | Low |
| `port/mp_modules/overlay_modssl_mbedtls.c` | Micropython module surface tests: import/init behavior, parameter checking, and return/error consistency. | Low |
| `port/mp_modules/overlay_vfs_posix_file.c` | Micropython module surface tests: import/init behavior, parameter checking, and return/error consistency. | Low |
| `port/multi_media/multimedia_runtime.c` | Stream/session state-machine tests: start-stop sequencing, buffer ownership, and timeout/retry behavior. | Medium |
| `port/multi_media/multimedia_wrap.cpp` | Stream/session state-machine tests: start-stop sequencing, buffer ownership, and timeout/retry behavior. | Medium |
| `port/multi_media/RtspServer.c` | Stream/session state-machine tests: start-stop sequencing, buffer ownership, and timeout/retry behavior. | Medium |
| `port/network/network_rt_smart.c` | Protocol and socket tests with fake transport: connect/reconnect paths, timeouts, and partial-read handling. | Medium |
| `port/omv/alloc/fb_alloc.c` | Memory behavior tests: alignment, zero-length, resize semantics, and stress/fuzz safety checks. | High |
| `port/omv/alloc/umm_malloc.c` | Memory behavior tests: alignment, zero-length, resize semantics, and stress/fuzz safety checks. | High |
| `port/omv/alloc/unaligned_memcpy.c` | Memory behavior tests: alignment, zero-length, resize semantics, and stress/fuzz safety checks. | High |
| `port/omv/alloc/xalloc.c` | Memory behavior tests: alignment, zero-length, resize semantics, and stress/fuzz safety checks. | High |
| `port/omv/common/array.c` | Data-structure and utility API tests: ordering, edge conditions, ownership/lifetime, and concurrency assumptions. | High |
| `port/omv/common/ff_wrapper.c` | Data-structure and utility API tests: ordering, edge conditions, ownership/lifetime, and concurrency assumptions. | High |
| `port/omv/common/mutex.c` | Data-structure and utility API tests: ordering, edge conditions, ownership/lifetime, and concurrency assumptions. | High |
| `port/omv/common/ringbuf.c` | Data-structure and utility API tests: ordering, edge conditions, ownership/lifetime, and concurrency assumptions. | High |
| `port/omv/ide_dbg.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/omv/ide_dbg_vo_wbc.c` | Compile/smoke test plus API contract tests for normal, boundary, and failure paths. | Low |
| `port/omv/imlib/agast.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/apriltag.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/bayer.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/binary.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/blob.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/bmp.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/clahe.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/collections.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/dmtx.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/draw.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/edge.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/eye.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/fast.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/fft.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/filter.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/fmath.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/font.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/framebuffer.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/freetype_wrap.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/fsort.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/gif.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/haar.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/hog.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/hough.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/imlib.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/integral.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/integral_mw.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/isp.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/jpeg.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/jpegd.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/kmeans.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/lab_tab.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/lbp.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/line.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/lodepng.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/lsd.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/mathop.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/mjpeg.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/orb.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/phasecorrelation.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/png.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/point.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/pool.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/ppm.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/qrcode.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/qsort.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/rainbow_tab.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/rectangle.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/selective_search.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/sincos_tab.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/stats.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/stereo.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/template.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/xyz_tab.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/yuv.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/imlib/zbar.c` | Golden-vector algorithm tests plus malformed input and size-boundary regression cases. | Medium |
| `port/omv/modules/py_clock.c` | Micropython binding tests for argument parsing, exception mapping, and object lifecycle. | Low |
| `port/omv/modules/py_helper.c` | Micropython binding tests for argument parsing, exception mapping, and object lifecycle. | Low |
| `port/omv/modules/py_image.c` | Micropython binding tests for argument parsing, exception mapping, and object lifecycle. | Low |
| `port/omv/modules/py_imageio.c` | Micropython binding tests for argument parsing, exception mapping, and object lifecycle. | Low |
| `resources/examples/01-Micropython-Basics/demo_crc16.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/01-Micropython-Basics/demo_files.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/01-Micropython-Basics/demo_fs_info.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/01-Micropython-Basics/demo_globals.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/01-Micropython-Basics/demo_json.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/01-Micropython-Basics/demo_logging.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/01-Micropython-Basics/demo_sha256.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/01-Micropython-Basics/demo_sys_info.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/01-Micropython-Basics/demo_thread.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/01-Micropython-Basics/demo_time.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/01-Micropython-Basics/demo_tree.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/01-Micropython-Basics/demo_view_mem.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/01-Micropython-Basics/demo_yield.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/01-Micropython-Basics/demo_yield_task.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/02-Media/acodec.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/02-Media/aec_playrec.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/02-Media/ai_rtsp.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/02-Media/audio_pdm.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/02-Media/audio.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/02-Media/mp4demuxer.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/02-Media/mp4muxer.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/02-Media/ogg.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/02-Media/rtsp_server.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/02-Media/uvc.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/02-Media/uvc_with_csc.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/02-Media/video_decoder.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/02-Media/video_encoder.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/02-Media/video_player.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/02-Media/virtual_wbc_rtsp.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/adc.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/dht.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/display_and_touch.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/ds18b20.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/fft.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/fpioa.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/i2c_24c32.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/i2c_mpu6050.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/i2c_slave.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/i2c_ssd1306.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/pin_irq.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/pin.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/pwm.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/pwm_servo.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/read_chipid.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/read_chip_temperature.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/reset.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/rtc.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/spi_lcd_show_custom_screen.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/spi_lcd_show.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/spi.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/timer.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/touch.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/touch_user.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/uart1.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/uart2.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/uart.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/wdt.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/03-Machine/ws2812.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/04-Cipher/ucryptolib_aes128_cbc.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/04-Cipher/ucryptolib_aes128_ctr.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/04-Cipher/ucryptolib_aes128_ecb_enc.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/04-Cipher/ucryptolib_aes128_ecb_inpl.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/04-Cipher/ucryptolib_aes128_ecb_into.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/04-Cipher/ucryptolib_aes128_ecb.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/04-Cipher/ucryptolib_aes256_cbc.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/04-Cipher/ucryptolib_aes256_ecb.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/04-Cipher/uhashlib_md5.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/04-Cipher/uhashlib_sha1.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/04-Cipher/uhashlib_sha256.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/body_seg.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/dynamic_gesture.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/eye_gaze.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/face_detection.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/face_detect_yunet.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/face_landmark.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/face_liveness_rgb.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/face_mesh.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/face_parse.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/face_pose.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/face_recognition_lite.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/face_recognition.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/face_registration_lite.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/face_registration.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/falldown_detect.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/finger_guessing.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/hand_detection.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/hand_keypoint_class.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/hand_keypoint_detection.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/hand_recognition.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/keyword_spotting.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/license_plate_det.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/license_plate_det_rec.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/license_plate_det_rec_yolo.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/license_plate_det_yolo.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/multi_kws.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/nanotracker.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/object_detect_yolov8n.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/ocr_det.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/ocr_rec.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/person_detection.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/person_keypoint_detect.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/puzzle_game.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/segment_yolov8n.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/self_learning.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/space_resize.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/tts_zh.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/yolo11n_obb.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/05-AI-Demo/yolov8n_obb.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/06-Display/display_debugger.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/06-Display/display_hdmi.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/06-Display/display_lcd.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/06-Display/display_nt35516.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/06-Display/display_virt.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/07-April-Tags/find_apriltags_3d_pose.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/07-April-Tags/find_apriltags.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/08-Codes/find_barcodes.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/08-Codes/find_datamatrices.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/08-Codes/find_qrcodes.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/09-Color-Tracking/automatic_grayscale_color_tracking.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/09-Color-Tracking/automatic_rgb565_color_tracking.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/09-Color-Tracking/black_grayscale_line_following.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/09-Color-Tracking/image_histogram_info.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/09-Color-Tracking/image_statistics_info.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/09-Color-Tracking/multi_color_code_tracking.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/09-Color-Tracking/single_color_code_tracking.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/10-Drawing/arrow_drawing.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/10-Drawing/circle_drawing.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/10-Drawing/cross_drawing.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/10-Drawing/ellipse_drawing.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/10-Drawing/flood_fill.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/10-Drawing/image_drawing_advanced.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/10-Drawing/image_drawing_alpha_blending_test.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/10-Drawing/image_drawing.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/10-Drawing/keypoints_drawing.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/10-Drawing/line_drawing.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/10-Drawing/rectangle_drawing.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/10-Drawing/text_drawing.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/11-Feature-Detection/edges.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/11-Feature-Detection/find_blobs.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/11-Feature-Detection/find_lines.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/11-Feature-Detection/find_rects.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/11-Feature-Detection/hog.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/11-Feature-Detection/lbp.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/11-Feature-Detection/linear_regression_fast.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/adaptive_histogram_equalization.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/blur_filter.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/color_binary_filter.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/color_light_removal.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/edge_filter.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/erode_and_dilate.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/gamma_correction.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/grayscale_bilateral_filter.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/grayscale_binary_filter.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/grayscale_light_removal.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/histogram_equalization.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/kernel_filters.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/lens_correction.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/linear_polar.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/log_polar.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/mean_adaptive_threshold_filter.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/mean_filter.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/median_adaptive_threshold_filter.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/median_filter.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/midpoint_adaptive_threshold_filter.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/midpoint_filter.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/mode_adaptive_threshold_filter.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/mode_filter.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/negative.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/perspective_and_rotation_correction.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/perspective_correction.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/rotation_correction.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/sharpen_filter.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/unsharp_filter.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/12-Image-Filters/vflip_hmirror_transpose.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/14-Socket/http_client.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/14-Socket/https_client2.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/14-Socket/https_client.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/14-Socket/http_server.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/14-Socket/network_lan.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/14-Socket/network_lte.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/14-Socket/network_wlan_ap.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/14-Socket/network_wlan_sta.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/14-Socket/tcp_client.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/14-Socket/tcp_server.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/14-Socket/udp_clinet.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/14-Socket/udp_multicast.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/14-Socket/udp_multicast_receiver_pc.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/14-Socket/udp_multicast_sender_pc.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/14-Socket/udp_server.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/15-LVGL/lvgl_demo.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/15-LVGL/lvgl_with_sensor.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/16-AI-Cube/ClassificationApp.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/16-AI-Cube/DataCollectionCamera.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/16-AI-Cube/DetectionApp.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/16-AI-Cube/MultiLabelApp.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/16-AI-Cube/OCR_Det.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/16-AI-Cube/SegmentationApp.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/16-AI-Cube/SelfLearningApp.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/17-Sensor/camera_auto_focus_lcd.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/17-Sensor/camera_dual_bind_hdmi.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/17-Sensor/camera_manual_exposure_lcd.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/17-Sensor/camera_mirror_flip.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/17-Sensor/camera_single_bind_hdmi.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/17-Sensor/camera_single_bind_lcd.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/17-Sensor/camera_single_show_hdmi.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/17-Sensor/camera_single_show_lcd.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/17-Sensor/camera_single_show_spi_lcd.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/17-Sensor/camera_snapshot_and_save.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/17-Sensor/camera_triple_bind_hdmi.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/18-NNCase/ai2d+kpu.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/18-NNCase/kpu.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/19-CloudPlatScripts/deploy_cls_image.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/19-CloudPlatScripts/deploy_cls_video.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/19-CloudPlatScripts/deploy_det_image.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/19-CloudPlatScripts/deploy_det_video.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/19-CloudPlatScripts/deploy_ml_image.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/19-CloudPlatScripts/deploy_ml_video.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/19-CloudPlatScripts/deploy_multl_image.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/19-CloudPlatScripts/deploy_multl_video.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/19-CloudPlatScripts/deploy_ocrdet_image.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/19-CloudPlatScripts/deploy_ocrdet_video.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/19-CloudPlatScripts/deploy_ocr_image.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/19-CloudPlatScripts/deploy_ocrrec_image.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/19-CloudPlatScripts/deploy_ocr_video.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/19-CloudPlatScripts/deploy_seg_image.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/19-CloudPlatScripts/deploy_seg_video.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/20-YOLO-Module-Examples/yolo11_cls_image.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/20-YOLO-Module-Examples/yolo11_cls_video.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/20-YOLO-Module-Examples/yolo11_det_image.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/20-YOLO-Module-Examples/yolo11_det_video.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/20-YOLO-Module-Examples/yolo11_obb_image.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/20-YOLO-Module-Examples/yolo11_obb_video.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/20-YOLO-Module-Examples/yolo11_seg_image.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/20-YOLO-Module-Examples/yolo11_seg_video.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/20-YOLO-Module-Examples/yolov5_cls_image.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/20-YOLO-Module-Examples/yolov5_cls_video.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/20-YOLO-Module-Examples/yolov5_det_image.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/20-YOLO-Module-Examples/yolov5_det_video.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/20-YOLO-Module-Examples/yolov5_seg_image.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/20-YOLO-Module-Examples/yolov5_seg_video.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/20-YOLO-Module-Examples/yolov8_cls_image.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/20-YOLO-Module-Examples/yolov8_cls_video.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/20-YOLO-Module-Examples/yolov8_det_image.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/20-YOLO-Module-Examples/yolov8_det_video.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/20-YOLO-Module-Examples/yolov8_obb_image.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/20-YOLO-Module-Examples/yolov8_obb_video.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/20-YOLO-Module-Examples/yolov8_seg_image.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/20-YOLO-Module-Examples/yolov8_seg_video.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/21-AI-With-Others/ai_lvgl.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/21-AI-With-Others/ai_multi_thread.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/21-AI-With-Others/ai_save_mp4.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/21-AI-With-Others/ai_uart.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/21-AI-With-Others/ai_uvc_hard_decode.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/21-AI-With-Others/ai_uvc_soft_decode.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/21-AI-With-Others/face_detect_yunet_from_mp4.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/22-Others/ai2d_for_affine_rotate.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/22-Others/kpu_run_fps.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/22-Others/read_image_for_display.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/22-Others/save_image.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/grayscale_find_blobs.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/grayscale_find_circles.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/grayscale_find_corners.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/grayscale_find_edges.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/grayscale_find_rectangle.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/grayscale_find_rectangle_with_corners.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/grayscale_threshold_binary.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/rgb888_adjust_exposure_fast.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/rgb888_adjust_exposure.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/rgb888_blackhat.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/rgb888_calc_histogram.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/rgb888_close.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/rgb888_dilate.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/rgb888_erode.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/rgb888_find_blobs.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/rgb888_find_circles.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/rgb888_find_corners.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/rgb888_find_edges.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/rgb888_find_rectangels.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/rgb888_find_rectangle_with_corners.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/rgb888_gaussian_blur.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/rgb888_gradient.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/rgb888_mean_blur.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/rgb888_open.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/rgb888_pnp_distance_from_corners_and_find_target.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/rgb888_pnp_distance_from_corners.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/rgb888_pnp_distance.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/rgb888_threshold_binary.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/rgb888_tophat.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/rgb888_undistort.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/rgb888_white_balance_gray_world_fast_ex.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/rgb888_white_balance_gray_world_fast.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/rgb888_white_balance_white_patch_ex.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/23-CV_Lite/rgb888_white_balance_white_patch.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/examples/99-HelloWorld/helloworld.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/libs/AI2D.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/libs/AIBase.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/libs/PipeLine.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/libs/PlatTasks.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/libs/Utils.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/libs/WBCRtsp.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/libs/YOLO.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/test_todo/cartoon_filter.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/test_todo/color_bilateral_filter.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/test_todo/find_circles.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/test_todo/keypoints.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/test_todo/linear_regression_robust.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `resources/test_todo/template_matching.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `tools/generate_ide_resource.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |
| `tools/get_git_version.py` | Python/MicroPython module tests: importability, syntax validity, API behavior, and boundary/error handling. | Medium |

Total source files covered: 621
Language breakdown: C=178, C++=52, Python=391

## Implemented gtests in this unit_test scaffold
- port/omv/common/array.c
- port/omv/common/ringbuf.c
- port/omv/alloc/unaligned_memcpy.c
- port/omv/common/mutex.c
- port/omv/common/ff_wrapper.c
- All-source catalog validation for C/C++/Python inventory

## Next implementation order
1. port/omv/common/* and port/omv/alloc/* remaining files
2. port/machine/* with HAL mocks
3. port/omv/imlib/* algorithm vectors
4. resources/**/*.py behavior tests on-device
