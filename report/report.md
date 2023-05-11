---
小组成员: 龙静毅 3190101088
---

# 多媒体技术实验报告

## 小组成员及分工

| 序号 | 学号 | 专业班级 | 姓名 | 性别 | 分工 |
| --- | --- | --- | --- | --- | --- |
| 1 | 3190101088 | 计科 1903 | 龙静毅 | 男 | 全部 |

## 项目简介

本次项目为实现一个类 JPEG 图像压缩、解压程序，给定指定格式的图像（目前支持 bmp 和 ppm），输出压缩之后的文件，随后可以使用同一程序对其进行解压，输出为指定格式的图像（目前支持 bmp）

全部开发在 Ubuntu 22.04 系统上进行，使用 `CMake` 进行项目管理，`gnu` 编译工具链进行编译，`gdb` 进行调试。除了 RGB 格式和 BMP 格式之间的映射使用了 github 上的[开源代码](https://github.com/marc-q/libbmp.git)之外，所有其他部分均由我们自行实现。最终我们的程序在[此开源数据集上](http://imagecompression.info/test_images/) 做了测试。

## 技术细节

压缩的过程主要分为颜色格式转换、下采样、DCT、量化、无损压缩四个部分。整个压缩流程只使用了基本的 STL 函数库，没有使用其他第三方库。


### 颜色格式转换

在这里我们使用 YUV 作为内部存储的颜色格式，他和 RGB 之间可以使用简单的线性变换得到，在此不再赘述。

### 下采样

在这里我们直接采用了和 JPEG 标准相同的下采样方法，即 4:2:0，对 Y 不做下采样，对 U 和 V 分别将相邻的四个像素取平均值，合成一个像素。

### DCT

#### 基础 DCT 算法

DCT 的本质是找到 $\mathbb{R}^{8 \times 8}$ 上的一组正交基 $c_ic_j^T$，然后将 $\mathbb{R}^{8 \times 8}$ 上任意向量在标准正交基下的坐标与此正交基下的坐标进行变换，其中正向的变换为 DCT，逆向的变换为 iDCT。设 DCT 变换前的向量为 $f \in \mathbb{R}^{8 \times 8}$，则：

$$
\begin{aligned}
    DCT(f)_{i, j} &= c_ic_j^T \cdot f \\
    &= tr((c_ic_j^T)^Tf) \\
    &= tr(c_jc_i^Tf) \\
    &= tr(c_i^Tfc_j) \\
    &= c_i^Tfc_j
\end{aligned}
$$

因此如果设 $C = [c_1, c_2, \cdots, c_8]$，则上式等同于：

$$DCT(f) = C^T f C$$

从而 DCT 变换可以使用矩阵乘法简单的实现。

#### 快速 DCT

首先注意到 2D-DCT 的过程等价与先对行向量依次进行 1D-DCT，再对列向量依次进行 1D-DCT，因此我们可以只考虑一维的情况。此时 DCT 可以简单地写成矩阵乘法的形式：

$$DCT(f) = C^T f$$

快速 DCT 的核心思想和快速傅里叶变换类似，即通过将 $C_n$ （其中 $n$ 为采样点的个数，对应于矩阵的阶数）分解为如下的形式：

$$C_n^T = \begin{bmatrix}
C_{n / 2}^T & \\
&D_{n / 2}
\end{bmatrix} \begin{bmatrix}
I_{n / 2} & J_{n / 2} \\
-J_{n / 2} & I_{n / 2}
\end{bmatrix}$$

其中 $J_{n / 2}$ 为如下形式的矩阵：
$$\begin{bmatrix}
& & & 1\\
& & 1 \\
&\vdots \\
1
\end{bmatrix}$$

这样后者可以采用简单的加减法实现，而前者的总体运算量不及直接进行 $C_n$ 的矩阵乘法。不仅如此，对 $C_{n / 2}$ 还可以进行进一步的分解，最终算法复杂度为 $O(n \log n)$ 级别。

具体来说，我们基于[此讲义](https://web.stanford.edu/class/ee398a/handouts/lectures/07-TransformCoding.pdf#page=30)上的示意图实现了快速 DCT，见 `src/Transformation/Transformation.cc`。

#### 二进制 DCT

快速 DCT 的主要性能瓶颈为浮点数乘法，二进制 DCT 则将快速 DCT 中涉及到的浮点数使用形如 $k / 2^n$ 形式的二进制浮点数替代，此时只需要进行简单的位移和加法操作就可以代替快速 DCT 中的乘除法了。我们基于[此论文](https://thanglong.ece.jhu.edu/Tran/Pub/bindct-IEEESP.pdf)实现了二进制 DCT，见 `src/Transformation/Transformation.cc`

### 量化

我们直接采取了 JPEG 量化的方法：

$$Q(F)_{i, j} = round(\frac{F_{i, j}}{Q_{i, j}})$$

其中 $Q$ 为量化表，对 $Y$ 和 $UV$ 不同。具体的量化表我们直接参考了 JPEG 标准

### 无损压缩

同样，我们参考了 JPEG 量化的方法，对 DCT 之后的交流分量采取了 RLC，而对直流分量采用 DPCM，再将这两者的结果分别进行熵编码。

#### RLC

在这里，我们使用 $(skip, nonzero)$ 对表示序列中接下来 $0$ 的个数和紧接着的非零值。在序列的开头，使用一个数 $n$ 表示序列的长度。在之后进行熵编码的过程中，$skip$ 和 $nonzero$ 是分开来压缩的，因为两者语义不同、取值范围不同，分开压缩可以尽可能减小每一部分的编码长度。

#### DPCM

我们采取了最简单的预测函数 $\tilde{f}_{i} = f_{i - 1}$，并且由于是无损压缩，我们并不做量化。因此 DPCM 的结果可以简单地写成：

$$DPCM(f)_{i} = \left\{\begin{aligned}
&f_i - f_{i - 1} &i \gt 0 \\
&f_i & i = 0
\end{aligned}\right.$$

#### 熵编码

熵编码为将一列数字进行压缩的通用方法。首先，将每一个数字写成 $(length, bits)$ 的形式，其中 $length$ 为表示该数字所需要的最小比特数，而 $bits$ 为具体的数。当 $bits$ 首位为 $1$ 时，为正数，否则为负数，其对应的值的绝对值为逐位取反后的结果。

将每个数的长度字段进行霍夫曼编码后写入，bits 直接写入。为了方便对位的操作，我们实现了 Bitstream 类，见 `src/LosslessCompress/BitStream.cc`

## 实验结果

我们在[此开源数据集](http://imagecompression.info/test_images/)上测试了我们的程序。测试结果如下：（注意，所使用的 PPM 图像均为二进制而非文本模式！）

| 图片名 | 分辨率 | 压缩前大小 | 压缩后大小 | 压缩比 | 误差 | 用时(s) |
| --- | --- | --- | --- | --- | --- | --- |
| artificial | 2048 x 3072 | 19M | 269K | 72.3 | 5.67 | 0.26 |
| big_building | 5424 x 7216 | 112M | 3.1M | 36.1 | 6.43 | 1.32 |
| big_tree | 4560 x 6096 | 80M | 1.8M | 44.4 | 8.59 | 0.91 |
| bridge | 4064 x 2752 | 32M | 916K | 35.8 | 7.95 | 0.35 |
| cathedral | 3008 x 2000 | 18M | 376K | 49.0 | 5.94 | 0.20 |
| deer | 2656 x 4048 | 31M | 500K | 63.5 | 8.64 | 0.33 |
| fireworks | 2352 x 3136 | 22M | 211K | 106.8 | 4.13 | 0.21 |
| flower_foveon | 1520 x 2272 | 9.9M | 87K | 116.5 | 4.21 | 0.09 |
| hdr | 2048 x 3072 | 19M | 186K | 104.6 | 3.84 | 0.26 |
| leaves_iso_1600 | 2000 x 3008 | 18M | 729K | 25.3 | 9.57 | 0.20 |
| leaves_iso_200 | 2000 x 3008 | 18M | 684K | 26.9 | 8.40 | 0.21 |
| nightshot_iso_100 | 2352 x 3136 | 22M | 253K | 89.0 | 4.66 | 0.22 |
| nightshot_iso_1600 | 2352 x 3136 | 22M | 438K | 51.4 | 7.85 | 0.23 |
| spider_web | 2848 x 4256 | 35M | 267K | 134.2 | 3.07 | 0.36 |


其中误差如下计算：

$$E = \frac{1}{3}\left(\sqrt{\frac{\sum_{i, j} (R_{i, j} - \tilde{R}_{i, j})^2}{row \times col}} + \sqrt{\frac{\sum_{i, j} (G_{i, j} - \tilde{G}_{i, j})^2}{row \times col}} + \sqrt{\frac{\sum_{i, j} (B_{i, j} - \tilde{B}_{i, j})^2}{row \times col}}\right)$$

具体的图片比较大，因此我们单独打包上传，见 `test_pics.zip`，其中文件命名格式如下：

1. `A.ppm` 为原始图片
2. `A.ppm.compressed` 为压缩后图片
3. `A.ppm.decompressed.bmp` 为解压后图片

## 参考文献

1. 使用的开源数据集：http://imagecompression.info/test_images/
2. 快速 DCT：https://web.stanford.edu/class/ee398a/handouts/lectures/07-TransformCoding.pdf#page=30
3. 二进制 DCT：https://thanglong.ece.jhu.edu/Tran/Pub/bindct-IEEESP.pdf

## 备注

此项目可以直接从 Github 上直接下载。

### 关于编译与运行

编译方式，在根目录下执行

```
mkdir build
cd build
cmake ..
make
```

将会生成两个可执行文件 `build/src/jpeger` 和 `build/src/benchmark`。前者将 `/data/image/test.bmp` 压缩为 `test.bmp.compressed`，之后解压为 `test.decompressed.bmp`，通过前者能够看到压缩比，后者则显示了压缩的损失。

由于我个人的习惯是不把第三方库放到代码仓库里面，而是通过 CMakeLists 文件来自行搜索和下载（~~3202 年了，C++ 还是没有统一的包管理库~~），编译的时候可能会因为网络问题而失败（最近因为不明原因，Github 连接非常不稳定）。如果在编译的时候遇到困难，麻烦助教直接联系我。

### 关于使用与测试

请使用[此链接](http://imagecompression.info/test_images/rgb8bit.zip)下载数据并解压到 `data/image/test/` 中，之后运行 `build/src/benchmark`

`data/config/test.json` 为配置文件，其中 `Transformation` 一项可以是 `basic` 或者 `BDCT` 或者 `FDCT`，分别为基于矩阵运算的 DCT、二进制 DCT、快速 DCT