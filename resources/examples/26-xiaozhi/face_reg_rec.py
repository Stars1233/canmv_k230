from libs.PipeLine import PipeLine
from libs.AIBase import AIBase
from libs.AI2D import Ai2d
from libs.Utils import *
import os,sys,ujson,gc,math
from media.media import *
import nncase_runtime as nn
import ulab.numpy as np
import image
import aidemo
import uos

# 自定义人脸检测任务类
class FaceDetApp(AIBase):
    def __init__(self,kmodel_path,model_input_size,anchors,confidence_threshold=0.25,nms_threshold=0.3,rgb888p_size=[1920,1080],debug_mode=0):
        super().__init__(kmodel_path,model_input_size,rgb888p_size,debug_mode)
        # kmodel路径
        self.kmodel_path=kmodel_path
        # 检测模型输入分辨率
        self.model_input_size=model_input_size
        # 置信度阈值
        self.confidence_threshold=confidence_threshold
        # nms阈值
        self.nms_threshold=nms_threshold
        self.anchors=anchors
        # sensor给到AI的图像分辨率，宽16字节对齐
        self.rgb888p_size=rgb888p_size
        # debug模式
        self.debug_mode=debug_mode
        # 实例化Ai2d，用于实现模型预处理
        self.ai2d=Ai2d(debug_mode)
        # 设置Ai2d的输入输出格式和类型
        self.ai2d.set_ai2d_dtype(nn.ai2d_format.NCHW_FMT,nn.ai2d_format.NCHW_FMT,np.uint8, np.uint8)

    # 配置预处理操作，这里使用了pad和resize，Ai2d支持crop/shift/pad/resize/affine，具体代码请打开/sdcard/app/libs/AI2D.py查看
    def config_preprocess(self,input_image_size=None):
        with ScopedTiming("set preprocess config",self.debug_mode > 0):
            # 初始化ai2d预处理配置，默认为sensor给到AI的尺寸，可以通过设置input_image_size自行修改输入尺寸
            ai2d_input_size=input_image_size if input_image_size else self.rgb888p_size
            top, bottom, left, right,_ =letterbox_pad_param(self.rgb888p_size,self.model_input_size) 
            self.ai2d.pad([0, 0, 0, 0, top, bottom, left, right], 0, [104, 117, 123])  # 填充边缘
            # 设置resize预处理
            self.ai2d.resize(nn.interp_method.tf_bilinear, nn.interp_mode.half_pixel)
            # 构建预处理流程,参数为预处理输入tensor的shape和预处理输出的tensor的shape
            self.ai2d.build([1,3,ai2d_input_size[1],ai2d_input_size[0]],[1,3,self.model_input_size[1],self.model_input_size[0]])

    # 自定义后处理，results是模型输出的array列表，这里使用了aidemo库的face_det_post_process接口
    def postprocess(self,results):
        with ScopedTiming("postprocess",self.debug_mode > 0):
            res = aidemo.face_det_post_process(self.confidence_threshold,self.nms_threshold,self.model_input_size[0],self.anchors,self.rgb888p_size,results)
            if len(res)==0:
                return res,res
            else:
                return res[0],res[1]

# 自定义人脸注册任务类
class FaceRegistrationApp(AIBase):
    def __init__(self,kmodel_path,model_input_size,rgb888p_size=[1920,1080],debug_mode=0):
        super().__init__(kmodel_path,model_input_size,rgb888p_size,debug_mode)
        # kmodel路径
        self.kmodel_path=kmodel_path
        # 人脸注册模型输入分辨率
        self.model_input_size=model_input_size
        # sensor给到AI的图像分辨率，宽16字节对齐
        self.rgb888p_size=[ALIGN_UP(rgb888p_size[0],16),rgb888p_size[1]]
        # debug模式
        self.debug_mode=debug_mode
        # 标准5官
        self.umeyama_args_112 = [
            38.2946 , 51.6963 ,
            73.5318 , 51.5014 ,
            56.0252 , 71.7366 ,
            41.5493 , 92.3655 ,
            70.7299 , 92.2041
        ]
        self.ai2d=Ai2d(debug_mode)
        self.ai2d.set_ai2d_dtype(nn.ai2d_format.NCHW_FMT,nn.ai2d_format.NCHW_FMT,np.uint8, np.uint8)

    # 配置预处理操作，这里使用了affine，Ai2d支持crop/shift/pad/resize/affine，具体代码请打开/sdcard/app/libs/AI2D.py查看
    def config_preprocess(self,landm,input_image_size=None):
        with ScopedTiming("set preprocess config",self.debug_mode > 0):
            ai2d_input_size=input_image_size if input_image_size else self.rgb888p_size
            # 计算affine矩阵，并设置仿射变换预处理
            affine_matrix = self.get_affine_matrix(landm)
            self.ai2d.affine(nn.interp_method.cv2_bilinear,0, 0, 127, 1,affine_matrix)
            # 构建预处理流程,参数为预处理输入tensor的shape和预处理输出的tensor的shape
            self.ai2d.build([1,3,ai2d_input_size[1],ai2d_input_size[0]],[1,3,self.model_input_size[1],self.model_input_size[0]])

    # 自定义后处理
    def postprocess(self,results):
        with ScopedTiming("postprocess",self.debug_mode > 0):
            return results[0][0]

    def svd22(self,a):
        # svd
        s = [0.0, 0.0]
        u = [0.0, 0.0, 0.0, 0.0]
        v = [0.0, 0.0, 0.0, 0.0]
        s[0] = (math.sqrt((a[0] - a[3]) ** 2 + (a[1] + a[2]) ** 2) + math.sqrt((a[0] + a[3]) ** 2 + (a[1] - a[2]) ** 2)) / 2
        s[1] = abs(s[0] - math.sqrt((a[0] - a[3]) ** 2 + (a[1] + a[2]) ** 2))
        v[2] = math.sin((math.atan2(2 * (a[0] * a[1] + a[2] * a[3]), a[0] ** 2 - a[1] ** 2 + a[2] ** 2 - a[3] ** 2)) / 2) if \
        s[0] > s[1] else 0
        v[0] = math.sqrt(1 - v[2] ** 2)
        v[1] = -v[2]
        v[3] = v[0]
        u[0] = -(a[0] * v[0] + a[1] * v[2]) / s[0] if s[0] != 0 else 1
        u[2] = -(a[2] * v[0] + a[3] * v[2]) / s[0] if s[0] != 0 else 0
        u[1] = (a[0] * v[1] + a[1] * v[3]) / s[1] if s[1] != 0 else -u[2]
        u[3] = (a[2] * v[1] + a[3] * v[3]) / s[1] if s[1] != 0 else u[0]
        v[0] = -v[0]
        v[2] = -v[2]
        return u, s, v

    def image_umeyama_112(self,src):
        # 使用Umeyama算法计算仿射变换矩阵
        SRC_NUM = 5
        SRC_DIM = 2
        src_mean = [0.0, 0.0]
        dst_mean = [0.0, 0.0]
        for i in range(0,SRC_NUM * 2,2):
            src_mean[0] += src[i]
            src_mean[1] += src[i + 1]
            dst_mean[0] += self.umeyama_args_112[i]
            dst_mean[1] += self.umeyama_args_112[i + 1]
        src_mean[0] /= SRC_NUM
        src_mean[1] /= SRC_NUM
        dst_mean[0] /= SRC_NUM
        dst_mean[1] /= SRC_NUM
        src_demean = [[0.0, 0.0] for _ in range(SRC_NUM)]
        dst_demean = [[0.0, 0.0] for _ in range(SRC_NUM)]
        for i in range(SRC_NUM):
            src_demean[i][0] = src[2 * i] - src_mean[0]
            src_demean[i][1] = src[2 * i + 1] - src_mean[1]
            dst_demean[i][0] = self.umeyama_args_112[2 * i] - dst_mean[0]
            dst_demean[i][1] = self.umeyama_args_112[2 * i + 1] - dst_mean[1]
        A = [[0.0, 0.0], [0.0, 0.0]]
        for i in range(SRC_DIM):
            for k in range(SRC_DIM):
                for j in range(SRC_NUM):
                    A[i][k] += dst_demean[j][i] * src_demean[j][k]
                A[i][k] /= SRC_NUM
        T = [[1, 0, 0], [0, 1, 0], [0, 0, 1]]
        U, S, V = self.svd22([A[0][0], A[0][1], A[1][0], A[1][1]])
        T[0][0] = U[0] * V[0] + U[1] * V[2]
        T[0][1] = U[0] * V[1] + U[1] * V[3]
        T[1][0] = U[2] * V[0] + U[3] * V[2]
        T[1][1] = U[2] * V[1] + U[3] * V[3]
        scale = 1.0
        src_demean_mean = [0.0, 0.0]
        src_demean_var = [0.0, 0.0]
        for i in range(SRC_NUM):
            src_demean_mean[0] += src_demean[i][0]
            src_demean_mean[1] += src_demean[i][1]
        src_demean_mean[0] /= SRC_NUM
        src_demean_mean[1] /= SRC_NUM
        for i in range(SRC_NUM):
            src_demean_var[0] += (src_demean_mean[0] - src_demean[i][0]) * (src_demean_mean[0] - src_demean[i][0])
            src_demean_var[1] += (src_demean_mean[1] - src_demean[i][1]) * (src_demean_mean[1] - src_demean[i][1])
        src_demean_var[0] /= SRC_NUM
        src_demean_var[1] /= SRC_NUM
        scale = 1.0 / (src_demean_var[0] + src_demean_var[1]) * (S[0] + S[1])
        T[0][2] = dst_mean[0] - scale * (T[0][0] * src_mean[0] + T[0][1] * src_mean[1])
        T[1][2] = dst_mean[1] - scale * (T[1][0] * src_mean[0] + T[1][1] * src_mean[1])
        T[0][0] *= scale
        T[0][1] *= scale
        T[1][0] *= scale
        T[1][1] *= scale
        return T

    def get_affine_matrix(self,sparse_points):
        # 获取affine变换矩阵
        with ScopedTiming("get_affine_matrix", self.debug_mode > 1):
            # 使用Umeyama算法计算仿射变换矩阵
            matrix_dst = self.image_umeyama_112(sparse_points)
            matrix_dst = [matrix_dst[0][0],matrix_dst[0][1],matrix_dst[0][2],
                          matrix_dst[1][0],matrix_dst[1][1],matrix_dst[1][2]]
            return matrix_dst

# 人脸注册任务类
class FaceRegAndRec:
    def __init__(self, rgb888p_size, debug_mode=0):
        
        # 人脸检测模型路径
        self.face_det_kmodel="/sdcard/examples/kmodel/face_detection_320.kmodel"
        # 人脸注册模型路径
        self.face_reg_kmodel="/sdcard/examples/kmodel/face_recognition.kmodel"
        # 其它参数
        anchors_path="/sdcard/examples/utils/prior_data_320.bin"
        self.database_dir="/sdcard/examples/utils/xiaozhi_db/"
        self.det_input_size=[320,320]
        self.reg_input_size=[112,112]
        self.confidence_threshold=0.5
        self.face_recognition_threshold = 0.75
        self.nms_threshold=0.2
        anchor_len=4200
        det_dim=4
        self.register_face_num = 0
        self.max_register_face = 5
        anchors = np.fromfile(anchors_path, dtype=np.float)
        anchors = anchors.reshape((anchor_len,det_dim))
        # anchors
        self.anchors=anchors
        # sensor给到AI的图像分辨率，宽16字节对齐
        self.rgb888p_size=rgb888p_size
        # debug_mode模式
        self.debug_mode=debug_mode
        self.register_db_data = []
        self.register_db_name = []
        self.face_det=FaceDetApp(self.face_det_kmodel,model_input_size=self.det_input_size,anchors=self.anchors,confidence_threshold=self.confidence_threshold,nms_threshold=self.nms_threshold, rgb888p_size=self.rgb888p_size, debug_mode=0)
        self.face_reg=FaceRegistrationApp(self.face_reg_kmodel,model_input_size=self.reg_input_size,rgb888p_size=self.rgb888p_size)
        self.face_det.config_preprocess()
        self.check_register_db()
        self.database_init()
        
    def database_init(self):
        
        print("database_init")
        db_file_list = os.listdir(self.database_dir)
        for db_file in db_file_list:
            if not db_file.endswith('.bin'):
                continue
            full_db_file = self.database_dir + db_file
            with open(full_db_file, 'rb') as f:
                data = f.read()
            feature = np.frombuffer(data, dtype=np.float)
            print(type(feature))
            print(feature)
            self.register_db_data.append(feature)
            name = db_file.split('.')[0]
            self.register_db_name.append(name)
            self.register_face_num += 1
        
    def database_search(self,feature):
        if self.register_face_num == 0:
            return -1, 'unknown', 0
        # 数据库查询
        v_id = -1
        v_score_max = 0.0
        # 将当前人脸特征归一化
        feature /= np.linalg.norm(feature)
        # 遍历当前人脸数据库，统计最高得分
        for i in range(self.register_face_num):
            db_feature = self.register_db_data[i]
            db_feature /= np.linalg.norm(db_feature)
            # 计算数据库特征与当前人脸特征相似度
            v_score = np.dot(feature, db_feature)/2 + 0.5
            if v_score > v_score_max:
                v_score_max = v_score
                v_id = i
        if v_id == -1:
            # 数据库中无人脸
            return -1, 'unknown', 0
        elif v_score_max < self.face_recognition_threshold:
            # 小于人脸识别阈值，未识别
            return -1, 'unknown', 0
        else:
            # 识别成功
            result = 0, self.register_db_name[v_id], v_score_max
            return result
                        
    def database_reset(self):
        print("database clearing...")
        self.db_name = []
        self.db_data = []
        self.register_face_num = 0
        print("database clear Done!")

    def recognize(self,input_np):
        # 执行人脸检测
        det_boxes,landms=self.face_det.run(input_np)
        recg_res = []
        results = []
        for landm in landms:
            # 针对每个人脸五官点，推理得到人脸特征，并计算特征在数据库中相似度
            self.face_reg.config_preprocess(landm)
            feature=self.face_reg.run(input_np)
            result, name, _= self.database_search(feature)
            recg_res.append(name)
            results.append(result)
        return det_boxes, recg_res, results
                
    def check_register_db(self):
        try:
            uos.stat(self.database_dir)
        except OSError:
            # 仅创建最后一级目录
            uos.mkdir(self.database_dir)
            print(f"创建目录：{self.database_dir}")
        except Exception as e:
            print(f"异常：{e}")
        
    def get_register_num(self):
        return self.register_face_num
    
    def clear_register_face(self):
        
        for item in uos.ilistdir(self.database_dir):
            item_name, _, *_ = item  # 解析项名称和类型
            item_path = f"{self.database_dir}/{item_name}"  # 拼接完整路径

            try:
                uos.remove(item_path)  # 删除文件
            except OSError as e:
                # 捕获文件删除失败（只读/占用/权限不足）
                error_msg += f"删除文件失败 {item_path}：{e}；"
                print(f"❌ 删除文件失败：{item_path}，原因：{e}")
        
        self.database_reset()

    # run函数
    def register(self, input_np, register_name):
        if(self.register_face_num >= self.max_register_face):
            return -1, "Max register is 5,please clear face first"
        
        self.face_det.config_preprocess(input_image_size=[input_np.shape[3],input_np.shape[2]])
        det_boxes,landms=self.face_det.run(input_np)
        if det_boxes:
            if det_boxes.shape[0] == 1:
                # 若是只检测到一张人脸，则将该人脸注册到数据库
                for landm in landms:
                    self.face_reg.config_preprocess(landm,input_image_size=[input_np.shape[3],input_np.shape[2]])
                    reg_result = self.face_reg.run(input_np)
                    with open(self.database_dir+'{}.bin'.format(register_name), "wb") as file:
                        file.write(reg_result.tobytes())
                        self.register_db_data.append(reg_result)
                        self.register_db_name.append(register_name)
                        self.register_face_num += 1
                        return 0, 'Success!'
            else:
                return -1, 'Only one person in a picture when you sign up'
        else:
            return -1, 'No person detected'

    def image2rgb888array(self,img):   #4维
        # 将Image转换为rgb888格式
        with ScopedTiming("fr_kpu_deinit",self.debug_mode > 0):
            img_data_rgb888=img.to_rgb888()
            # hwc,rgb888
            img_hwc=img_data_rgb888.to_numpy_ref()
            shape=img_hwc.shape
            img_tmp = img_hwc.reshape((shape[0] * shape[1], shape[2]))
            img_tmp_trans = img_tmp.transpose()
            img_res=img_tmp_trans.copy()
            # chw,rgb888
            img_return=img_res.reshape((1,shape[2],shape[0],shape[1]))
        return  img_return


