from libs.PipeLine import ScopedTiming
from libs.AIBase import AIBase
from libs.AI2D import Ai2d
from media.media import *
import nncase_runtime as nn
import ulab.numpy as np
import aicube
import config

# ============================================================
# Hand detection
# ============================================================
class HandDetApp(AIBase):
    def __init__(self, display_size, debug_mode=0):
        rgb888p_size = [ALIGN_UP(config.RGB888P_SIZE[0], 16), config.RGB888P_SIZE[1]]
        super().__init__(config.HAND_DET_KMODEL, config.DET_INPUT_SIZE,
                         rgb888p_size, debug_mode)
        self.kmodel_path = config.HAND_DET_KMODEL
        self.model_input_size = config.DET_INPUT_SIZE
        self.labels = config.LABELS
        self.anchors = config.ANCHORS
        self.strides = config.STRIDES
        self.confidence_threshold = config.CONFIDENCE_THRESHOLD
        self.nms_threshold = config.NMS_THRESHOLD
        self.nms_option = config.NMS_OPTION
        self.rgb888p_size = rgb888p_size
        self.display_size = [ALIGN_UP(display_size[0], 16), display_size[1]]
        self.debug_mode = debug_mode
        self.ai2d = Ai2d(debug_mode)
        self.ai2d.set_ai2d_dtype(nn.ai2d_format.NCHW_FMT, nn.ai2d_format.NCHW_FMT,
                                 np.uint8, np.uint8)

    def config_preprocess(self, input_image_size=None):
        with ScopedTiming("set preprocess config", self.debug_mode > 0):
            ai2d_input_size = input_image_size if input_image_size else self.rgb888p_size
            top, bottom, left, right = self.get_padding_param()
            self.ai2d.pad([0, 0, 0, 0, top, bottom, left, right], 0,
                          config.DET_PAD_COLOR)
            self.ai2d.resize(nn.interp_method.tf_bilinear, nn.interp_mode.half_pixel)
            self.ai2d.build([1, 3, ai2d_input_size[1], ai2d_input_size[0]],
                            [1, 3, self.model_input_size[1], self.model_input_size[0]])

    def postprocess(self, results):
        with ScopedTiming("postprocess", self.debug_mode > 0):
            dets = aicube.anchorbasedet_post_process(
                results[0], results[1], results[2], self.model_input_size,
                self.rgb888p_size, self.strides, len(self.labels),
                self.confidence_threshold, self.nms_threshold,
                self.anchors, self.nms_option)
            return dets

    def get_padding_param(self):
        # Letterbox the frame into the square model input, keeping aspect ratio.
        dst_w, dst_h = self.model_input_size
        src_w, src_h = self.rgb888p_size
        ratio = min(dst_w / src_w, dst_h / src_h)
        dw = (dst_w - int(ratio * src_w)) / 2
        dh = (dst_h - int(ratio * src_h)) / 2
        top = int(round(dh - 0.1))
        bottom = int(round(dh + 0.1))
        left = int(round(dw - 0.1))
        right = int(round(dw + 0.1))
        return top, bottom, left, right



# ============================================================
# Hand keypoint detection
# ============================================================
class HandKPDetApp(AIBase):
    def __init__(self, display_size, debug_mode=0):
        rgb888p_size = [ALIGN_UP(config.RGB888P_SIZE[0], 16), config.RGB888P_SIZE[1]]
        super().__init__(config.HAND_KP_KMODEL, config.KP_INPUT_SIZE,
                         rgb888p_size, debug_mode)
        self.kmodel_path = config.HAND_KP_KMODEL
        self.model_input_size = config.KP_INPUT_SIZE
        self.rgb888p_size = rgb888p_size
        self.display_size = [ALIGN_UP(display_size[0], 16), display_size[1]]
        self.crop_params = []
        self.debug_mode = debug_mode

        self._sx = self.display_size[0] / self.rgb888p_size[0]
        self._sy = self.display_size[1] / self.rgb888p_size[1]

        self.ai2d = Ai2d(debug_mode)
        self.ai2d.set_ai2d_dtype(nn.ai2d_format.NCHW_FMT, nn.ai2d_format.NCHW_FMT, np.uint8, np.uint8)

    def config_preprocess(self, det, input_image_size=None):
        with ScopedTiming("set preprocess config", self.debug_mode > 0):
            ai2d_input_size = input_image_size if input_image_size else self.rgb888p_size
            self.crop_params = self.get_crop_param(det)
            self.ai2d.crop(self.crop_params[0], self.crop_params[1],
                           self.crop_params[2], self.crop_params[3])
            self.ai2d.resize(nn.interp_method.tf_bilinear, nn.interp_mode.half_pixel)
            self.ai2d.build([1, 3, ai2d_input_size[1], ai2d_input_size[0]],
                            [1, 3, self.model_input_size[1], self.model_input_size[0]])

    def postprocess(self, results):
        with ScopedTiming("postprocess", self.debug_mode > 0):
            results = results[0].reshape(results[0].shape[0] * results[0].shape[1])
            results_show = np.zeros(results.shape, dtype=np.int16)
            results_show[0::2] = results[0::2] * self.crop_params[3] + self.crop_params[0]
            results_show[1::2] = results[1::2] * self.crop_params[2] + self.crop_params[1]

            self.last_sensor_box = (int(np.min(results_show[0::2])),
                                    int(np.min(results_show[1::2])),
                                    int(np.max(results_show[0::2])),
                                    int(np.max(results_show[1::2])))
            results_show[0::2] = results_show[0::2] * self._sx
            results_show[1::2] = results_show[1::2] * self._sy

            # return hand keypoint crop box  (21 points)
            return results_show

    def get_crop_param(self, det_box):
        # Crop the hand image
        x1, y1, x2, y2 = det_box[2], det_box[3], det_box[4], det_box[5]
        length = max(int(x2 - x1), int(y2 - y1)) / 2
        cx = (x1 + x2) / 2
        cy = (y1 + y2) / 2
        margin = config.KP_CROP_RATIO * length
        x1_kp = int(max(0, cx - margin))
        y1_kp = int(max(0, cy - margin))
        x2_kp = int(min(self.rgb888p_size[0] - 1, cx + margin))
        y2_kp = int(min(self.rgb888p_size[1] - 1, cy + margin))

        return [x1_kp, y1_kp, int(x2_kp - x1_kp + 1), int(y2_kp - y1_kp + 1)]


# ============================================================
# main AI pipeline: hand detection + keypoint detection.
# ============================================================
class HandKeyPointDet:
    def __init__(self, display_size, debug_mode=0):
        self.rgb888p_size = [ALIGN_UP(config.RGB888P_SIZE[0], 16), config.RGB888P_SIZE[1]]
        self.display_size = [ALIGN_UP(display_size[0], 16), display_size[1]]
        self.hand_det = HandDetApp(display_size, debug_mode)
        self.hand_kp = HandKPDetApp(display_size, debug_mode)
        self.hand_det.config_preprocess()
        self.frame_i = 0
        self.tracked = []

    def _keep(self, det_box):
        # Reject boxes too small to be a hand, or clipped by the frame edge.
        W, H = self.rgb888p_size
        x1, x2 = det_box[2], det_box[4]
        w = int(x2 - x1)
        if int(det_box[5] - det_box[3]) < config.MIN_HAND_H_RATIO * H:
            return False
        for (max_w_ratio, edge) in config.EDGE_FILTERS:
            if w < max_w_ratio * W and (x1 < edge * W or x2 > (1.0 - edge) * W):
                return False
        return True

    def _in_frame(self, sb):
        # Reject hands clipped by the border. The keypoint model always
        # returns 21 points for whatever is in the crop, even background,
        # so a half-exited hand produces a plausible-looking ghost.
        W, H = self.rgb888p_size
        mx = config.EXIT_MARGIN * W
        my = config.EXIT_MARGIN * H
        x1, y1, x2, y2 = sb
        return (x1 > mx and y1 > my
                and x2 < W - 1 - mx and y2 < H - 1 - my)

    def _next_box(self, sb):
        W, H = self.rgb888p_size
        x1, y1, x2, y2 = sb
        w, h = x2 - x1, y2 - y1
        if h < config.MIN_HAND_H_RATIO * H * 0.7 or w < 16 or w > 0.9 * W:
            return None
        mx = w * config.TRACK_EXPAND
        my = h * config.TRACK_EXPAND
        return [0, 1.0,
                max(0, x1 - mx), max(0, y1 - my),
                min(W - 1, x2 + mx), min(H - 1, y2 + my)]

    def run(self, input_np):
        self.frame_i += 1
        if self.tracked and (self.frame_i % config.DET_INTERVAL) != 0:
            # Fast path: skip the detector, reuse the tracked boxes.
            src = self.tracked
        else:
            # Full detection: ground truth. Rebuilds the cache from scratch
            src = [b for b in self.hand_det.run(input_np) if self._keep(b)]

        boxes, hand_res, nxt = [], [], []
        for det_box in src:
            self.hand_kp.config_preprocess(det_box)
            res = self.hand_kp.run(input_np)
            sb = self.hand_kp.last_sensor_box
            if not self._in_frame(sb):
                continue
            boxes.append(det_box)
            hand_res.append(res)
            nb = self._next_box(sb)
            if nb:
                nxt.append(nb)
        self.tracked = nxt          # empty cache -> next frame detects again
        return boxes, hand_res