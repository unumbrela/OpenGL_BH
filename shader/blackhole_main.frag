#version 330 core

const float PI = 3.14159265359; // 圆周率常量
const float EPSILON = 0.0001;  // 用于浮点比较的极小值
const float INFINITY = 1000000.0; // 近似表示的无穷大值

out vec4 fragColor; // 输出的片段颜色

// Uniform变量声明
uniform vec2 resolution; // 视口分辨率（像素）
uniform float mouseX;    // 鼠标X位置
uniform float mouseY;    // 鼠标Y位置

uniform float time; // 经过的时间（秒）
uniform samplerCube galaxy; // 天空盒立方体纹理
uniform sampler2D colorMap; // 颜色贴图

// 摄像机视角控制参数
uniform float frontView = 0.0;       // 前视图开关
uniform float topView = 0.0;         // 顶视图开关
uniform float cameraRoll = 0.0;      // 摄像机滚动角度

// 渲染控制参数
uniform float gravatationalLensing = 1.0; // 引力透镜效果开关
uniform float renderBlackHole = 1.0;      // 黑洞渲染开关
uniform float mouseControl = 0.0;          // 鼠标控制开关
uniform float fovScale = 1.0;              // 视野缩放比例

// 吸积盘（accretion disk）相关参数
uniform float adiskEnabled = 1.0;      // 吸积盘启用开关
uniform float adiskParticle = 1.0;     // 吸积盘颗粒启用开关
uniform float adiskHeight = 0.2;       // 吸积盘高度
uniform float adiskLit = 0.5;          // 吸积盘亮度
uniform float adiskDensityV = 1.0;     // 吸积盘垂直密度
uniform float adiskDensityH = 1.0;     // 吸积盘水平密度
uniform float adiskNoiseScale = 1.0;   // 吸积盘噪声缩放
uniform float adiskNoiseLOD = 5.0;     // 吸积盘噪声层级
uniform float adiskSpeed = 0.5;        // 吸积盘旋转速度

// 圆环结构体定义
struct Ring {
  vec3 center;        // 圆环中心位置
  vec3 normal;        // 圆环法线方向
  float innerRadius;  // 圆环内半径
  float outerRadius;  // 圆环外半径
  float rotateSpeed;  // 圆环旋转速度
};

///----
/// Simplex 3D Noise 实现
/// 作者: Ian McEwan, Ashima Arts

// 排列函数，用于生成随机数序列
vec4 permute(vec4 x) { return mod(((x * 34.0) + 1.0) * x, 289.0); }

// 泰勒逆平方根近似，用于归一化梯度
vec4 taylorInvSqrt(vec4 r) { return 1.79284291400159 - 0.85373472095314 * r; }

// Simplex噪声函数
float snoise(vec3 v) {
  const vec2 C = vec2(1.0 / 6.0, 1.0 / 3.0);
  const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);

  // 第一个顶点
  vec3 i = floor(v + dot(v, C.yyy));
  vec3 x0 = v - i + dot(i, C.xxx);

  // 其他顶点
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min(g.xyz, l.zxy);
  vec3 i2 = max(g.xyz, l.zxy);

  // 计算其他三个顶点的位置
  vec3 x1 = x0 - i1 + 1.0 * C.xxx;
  vec3 x2 = x0 - i2 + 2.0 * C.xxx;
  vec3 x3 = x0 - 1.0 + 3.0 * C.xxx;

  // 生成排列索引
  i = mod(i, 289.0);
  vec4 p = permute(permute(permute(i.z + vec4(0.0, i1.z, i2.z, 1.0)) + i.y +
                           vec4(0.0, i1.y, i2.y, 1.0)) +
                   i.x + vec4(0.0, i1.x, i2.x, 1.0));

  // 生成梯度
  float n_ = 1.0 / 7.0; // N=7
  vec3 ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z); // mod(p,N*N)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_); // mod(j,N)

  vec4 x = x_ * ns.x + ns.yyyy;
  vec4 y = y_ * ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4(x.xy, y.xy);
  vec4 b1 = vec4(x.zw, y.zw);

  vec4 s0 = floor(b0) * 2.0 + 1.0;
  vec4 s1 = floor(b1) * 2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
  vec4 a1 = b1.xzyw + s1.xzyw * sh.zzww;

  vec3 p0 = vec3(a0.xy, h.x);
  vec3 p1 = vec3(a0.zw, h.y);
  vec3 p2 = vec3(a1.xy, h.z);
  vec3 p3 = vec3(a1.zw, h.w);

  // 归一化梯度
  vec4 norm =
      taylorInvSqrt(vec4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

  // 混合最终噪声值
  vec4 m =
      max(0.6 - vec4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
  m = m * m;
  return 42.0 *
         dot(m * m, vec4(dot(p0, x0), dot(p1, x1), dot(p2, x2), dot(p3, x3)));
}
///----

// 计算光线与圆环的距离，如果有交点则返回距离，否则返回-1.0
float ringDistance(vec3 rayOrigin, vec3 rayDir, Ring ring) {
  float denominator = dot(rayDir, ring.normal); // 分母：光线方向与圆环法线的点积
  float constant = -dot(ring.center, ring.normal); // 常数项
  if (abs(denominator) < EPSILON) { // 如果分母接近零，表示光线平行于圆环
    return -1.0;
  } else {
    float t = -(dot(rayOrigin, ring.normal) + constant) / denominator; // 求解t值
    if (t < 0.0) { // 如果t小于0，交点在光线起点之前
      return -1.0;
    }

    vec3 intersection = rayOrigin + t * rayDir; // 计算交点位置

    // 计算交点到圆环中心的距离
    float d = length(intersection - ring.center);
    if (d >= ring.innerRadius && d <= ring.outerRadius) { // 判断是否在圆环范围内
      return t;
    }
    return -1.0; // 不在圆环范围内
  }
}

// 从方向向量获取全景颜色
vec3 panoramaColor(sampler2D tex, vec3 dir) {
  // 将方向向量转换为UV坐标
  vec2 uv = vec2(0.5 - atan(dir.z, dir.x) / PI * 0.5, 0.5 - asin(dir.y) / PI);
  return texture(tex, uv).rgb; // 采样纹理颜色
}

// 计算加速度，用于引力透镜效果
vec3 accel(float h2, vec3 pos) {
  float r2 = dot(pos, pos); // 位置向量的平方
  float r5 = pow(r2, 2.5);  // r的5次方
  vec3 acc = -1.5 * h2 * pos / r5 * 1.0; // 计算加速度
  return acc;
}

// 根据轴和角度生成四元数
vec4 quadFromAxisAngle(vec3 axis, float angle) {
  vec4 qr;
  float half_angle = (angle * 0.5) * 3.14159 / 180.0; // 将角度转换为弧度并取一半
  qr.x = axis.x * sin(half_angle);
  qr.y = axis.y * sin(half_angle);
  qr.z = axis.z * sin(half_angle);
  qr.w = cos(half_angle);
  return qr;
}

// 计算四元数的共轭
vec4 quadConj(vec4 q) { return vec4(-q.x, -q.y, -q.z, q.w); }

// 四元数相乘
vec4 quat_mult(vec4 q1, vec4 q2) {
  vec4 qr;
  qr.x = (q1.w * q2.x) + (q1.x * q2.w) + (q1.y * q2.z) - (q1.z * q2.y);
  qr.y = (q1.w * q2.y) - (q1.x * q2.z) + (q1.y * q2.w) + (q1.z * q2.x);
  qr.z = (q1.w * q2.z) + (q1.x * q2.y) - (q1.y * q2.x) + (q1.z * q2.w);
  qr.w = (q1.w * q2.w) - (q1.x * q2.x) - (q1.y * q2.y) - (q1.z * q2.z);
  return qr;
}

// 使用四元数旋转向量
vec3 rotateVector(vec3 position, vec3 axis, float angle) {
  vec4 qr = quadFromAxisAngle(axis, angle);      // 生成旋转四元数
  vec4 qr_conj = quadConj(qr);                   // 计算四元数的共轭
  vec4 q_pos = vec4(position, 0.0);             // 将向量转换为四元数形式

  vec4 q_tmp = quat_mult(qr, q_pos);            // q1 * q_pos
  qr = quat_mult(q_tmp, qr_conj);               // (q1 * q_pos) * q_conj

  return vec3(qr.x, qr.y, qr.z);                // 返回旋转后的向量
}

#define IN_RANGE(x, a, b) (((x) > (a)) && ((x) < (b))) // 判断x是否在(a, b)范围内

// 将笛卡尔坐标转换为球面坐标（rho, phi, theta）
void cartesianToSpherical(in vec3 xyz, out float rho, out float phi,
                          out float theta) {
  rho = sqrt((xyz.x * xyz.x) + (xyz.y * xyz.y) + (xyz.z * xyz.z)); // 半径
  phi = asin(xyz.y / rho);                                       // 仰角
  theta = atan(xyz.z, xyz.x);                                    // 方位角
}

// 从笛卡尔坐标转换为球面坐标（rho, theta, phi）
vec3 toSpherical(vec3 p) {
  float rho = sqrt((p.x * p.x) + (p.y * p.y) + (p.z * p.z)); // 半径
  float theta = atan(p.z, p.x);                              // 方位角
  float phi = asin(p.y / rho);                               // 仰角
  return vec3(rho, theta, phi);                               // 返回球面坐标
}

// 另一种球面坐标转换，带有缩放和偏移
vec3 toSpherical2(vec3 pos) {
  vec3 radialCoords;
  radialCoords.x = length(pos) * 1.5 + 0.55;               // 缩放半径
  radialCoords.y = atan(-pos.x, -pos.z) * 1.5;            // 缩放方位角
  radialCoords.z = abs(pos.y);                             // 绝对仰角
  return radialCoords;                                     // 返回调整后的球面坐标
}

// 计算并设置圆环的颜色
void ringColor(vec3 rayOrigin, vec3 rayDir, Ring ring, inout float minDistance,
               inout vec3 color) {
  float distance = ringDistance(rayOrigin, normalize(rayDir), ring); // 计算光线与圆环的距离
  if (distance >= EPSILON && distance < minDistance &&
      distance <= length(rayDir) + EPSILON) { // 判断是否满足条件
    minDistance = distance; // 更新最小距离

    vec3 intersection = rayOrigin + normalize(rayDir) * minDistance; // 计算交点
    vec3 ringColor;

    {
      float dist = length(intersection); // 计算交点距离

      float v = clamp((dist - ring.innerRadius) /
                          (ring.outerRadius - ring.innerRadius),
                      0.0, 1.0); // 计算归一化距离

      vec3 base = cross(ring.normal, vec3(0.0, 0.0, 1.0)); // 计算基准向量
      float angle = acos(dot(normalize(base), normalize(intersection))); // 计算角度
      if (dot(cross(base, intersection), ring.normal) < 0.0)
        angle = -angle; // 根据方向调整角度

      float u = 0.5 - 0.5 * angle / PI; // 计算U坐标
      // HACK：动态旋转
      u += time * ring.rotateSpeed;

      vec3 color = vec3(0.0, 0.5, 0.0); // 圆环基础颜色
      // HACK：设置透明度
      float alpha = 0.5;
      ringColor = vec3(color); // 设置圆环颜色
    }

    color += ringColor; // 叠加颜色
  }
}

// 构建视图矩阵
mat3 lookAt(vec3 origin, vec3 target, float roll) {
  vec3 rr = vec3(sin(roll), cos(roll), 0.0); // 计算滚动向量
  vec3 ww = normalize(target - origin);      // 计算视线向量
  vec3 uu = normalize(cross(ww, rr));       // 计算右向量
  vec3 vv = normalize(cross(uu, ww));       // 计算上向量

  return mat3(uu, vv, ww); // 返回视图矩阵
}

// 计算向量的平方长度
float sqrLength(vec3 a) { return dot(a, a); }

// 计算并设置吸积盘的颜色
void adiskColor(vec3 pos, inout vec3 color, inout float alpha) {
  float innerRadius = 2.6; // 吸积盘内半径
  float outerRadius = 12.0; // 吸积盘外半径

  // 密度随距离增加线性减少
  float density = max(
      0.0, 1.0 - length(pos.xyz / vec3(outerRadius, adiskHeight, outerRadius)));
  if (density < 0.001) { // 密度过低则跳过
    return;
  }

  density *= pow(1.0 - abs(pos.y) / adiskHeight, adiskDensityV); // 根据高度调整密度

  // 当半径小于内稳定轨道时，密度设为0
  density *= smoothstep(innerRadius, innerRadius * 1.1, length(pos));

  // 当密度非常小时，避免不必要的计算
  if (density < 0.001) {
    return;
  }

  vec3 sphericalCoord = toSpherical(pos); // 转换为球面坐标

  // 缩放rho和phi，使颗粒在视觉上具有正确的比例
  sphericalCoord.y *= 2.0;
  sphericalCoord.z *= 4.0;

  density *= 1.0 / pow(sphericalCoord.x, adiskDensityH); // 根据rho调整密度
  density *= 16000.0; // 缩放密度值

  if (adiskParticle < 0.5) { // 如果颗粒模式关闭
    color += vec3(0.0, 1.0, 0.0) * density * 0.02; // 添加绿色颗粒
    return;
  }

  float noise = 1.0;
  for (int i = 0; i < int(adiskNoiseLOD); i++) { // 多级噪声叠加
    noise *= 0.5 * snoise(sphericalCoord * pow(i, 2) * adiskNoiseScale) + 0.5;
    if (i % 2 == 0) {
      sphericalCoord.y += time * adiskSpeed; // 动态调整y坐标
    } else {
      sphericalCoord.y -= time * adiskSpeed;
    }
  }

  vec3 dustColor =
      texture(colorMap, vec2(sphericalCoord.x / outerRadius, 0.5)).rgb; // 采样颜色贴图

  color += density * adiskLit * dustColor * alpha * abs(noise); // 叠加吸积盘颜色
}

// 光线追踪计算颜色
vec3 traceColor(vec3 pos, vec3 dir) {
  vec3 color = vec3(0.0); // 初始颜色为黑色
  float alpha = 1.0;       // 初始透明度

  float STEP_SIZE = 0.1; // 光线步长
  dir *= STEP_SIZE;      // 缩放方向向量

  // 初始值
  vec3 h = cross(pos, dir); // 计算角动量
  float h2 = dot(h, h);     // 角动量平方

  for (int i = 0; i < 300; i++) { // 最大迭代次数
    if (renderBlackHole > 0.5) { // 如果黑洞渲染开启
      // 如果引力透镜效果开启
      if (gravatationalLensing > 0.5) {
        vec3 acc = accel(h2, pos); // 计算加速度
        dir += acc; // 更新方向
      }

      // 如果到达事件视界，返回当前颜色
      if (dot(pos, pos) < 1.0) {
        return color;
      }

      float minDistance = INFINITY; // 初始化最小距离

      if (false) { // 预留代码块，可用于添加圆环渲染
        Ring ring;
        ring.center = vec3(0.0, 0.05, 0.0);
        ring.normal = vec3(0.0, 1.0, 0.0);
        ring.innerRadius = 2.0;
        ring.outerRadius = 6.0;
        ring.rotateSpeed = 0.08;
        ringColor(pos, dir, ring, minDistance, color);
      } else {
        if (adiskEnabled > 0.5) { // 如果吸积盘渲染开启
          adiskColor(pos, color, alpha); // 计算吸积盘颜色
        }
      }
    }

    pos += dir; // 更新位置
  }

  // 采样天空盒颜色
  dir = rotateVector(dir, vec3(0.0, 1.0, 0.0), time); // 旋转方向向量
  color += texture(galaxy, dir).rgb * alpha; // 叠加天空盒颜色
  return color; // 返回最终颜色
}

void main() {
  mat3 view; // 视图矩阵

  vec3 cameraPos; // 摄像机位置
  if (mouseControl > 0.5) { // 如果启用鼠标控制
    vec2 mouse = clamp(vec2(mouseX, mouseY) / resolution.xy, 0.0, 1.0) - 0.5; // 归一化鼠标坐标
    cameraPos = vec3(-cos(mouse.x * 10.0) * 15.0, mouse.y * 30.0,
                     sin(mouse.x * 10.0) * 15.0); // 根据鼠标位置计算摄像机位置

  } else if (frontView > 0.5) { // 前视图
    cameraPos = vec3(10.0, 1.0, 10.0);
  } else if (topView > 0.5) { // 顶视图
    cameraPos = vec3(15.0, 15.0, 0.0);
  } else { // 默认动态视角
    cameraPos = vec3(-cos(time * 0.1) * 15.0, sin(time * 0.1) * 15.0,
                     sin(time * 0.1) * 15.0);
  }

  vec3 target = vec3(0.0, 0.0, 0.0); // 摄像机目标位置
  view = lookAt(cameraPos, target, radians(cameraRoll)); // 构建视图矩阵

  vec2 uv = gl_FragCoord.xy / resolution.xy - vec2(0.5); // 标准化片段坐标
  uv.x *= resolution.x / resolution.y; // 修正纵横比

  vec3 dir = normalize(vec3(-uv.x * fovScale, uv.y * fovScale, 1.0)); // 计算光线方向
  vec3 pos = cameraPos; // 初始化光线起点
  dir = view * dir; // 应用视图变换

  fragColor.rgb = traceColor(pos, dir); // 计算片段颜色
}
