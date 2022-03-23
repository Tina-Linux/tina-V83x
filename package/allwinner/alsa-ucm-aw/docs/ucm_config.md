# ALSA UCM Configurations

此文档介绍了 alsa-ucm-aw-configs 中 UCM 配置文件的语法及约定

## 目录结构

UCM 配置文件放置在 ucm_configs 目录下, 按芯片级方案名字分类 (r16, r40, ...);
在芯片级方案目录下, 按板级方案分类 (r16-dm, r16-parrot, ...), 并有一个 Default
目录用于放置该芯片级方案默认的配置文件

alsa-ucm-aw-configs 在编译时, 会根据当前芯片级方案的名字将对应目录下的配置文件拷
贝到小机端的 /usr/share/alsa/ucm 目录; 若当前芯片级方案的配置文件不存在, 则会拷贝
Default 目录下的默认配置文件

```
ucm_configs
├── r16
│   ├── r16-dm
│   │   ├── AC108
│   │   │   ├── AC108.conf
│   │   │   └── Record.conf
│   │   └── audiocodec
│   │       ├── audiocodec.conf
│   │       └── Play.conf
│   ├── r16-parrot
│   │   └── audiocodec
│   │       ├── audiocodec.conf
│   │       ├── Play.conf
│   │       └── Record.conf
│   └── Default
│       └── audiocodec
│           ├── audiocodec.conf
│           ├── Play.conf
│           └── Record.conf
└── r40
    ├── r40-m2ultra
    │   └── audiocodec
    │       ├── audiocodec.conf
    │       ├── Play.conf
    │       └── Record.conf
    └── Default
        └── audiocodec
            ├── audiocodec.conf
            ├── Play.conf
            └── Record.conf
```

## UCM 配置文件

一份完整的 UCM 配置文件包括以下内容: card, verb, device, modifier (可选). 想要使
用某个 ALSA 设备并配置它的控件, 需要指定 1 个 card + 1 个 verb + 1 个或以上的
device + 0 个或以上的 modifier.

### card

表示声卡, 每个 card 单独一个目录, 每个方案需要包含一个或以上的 card, 如 r16-dm
下的 "AC108" 和 "audiocodec" 分别为两个不同的 card

### verb

表示使用案例 (use case), 表明某个特定的使用场景, 每个 card 需要包含一个或以上的
verb, 如 r16-parrot 的 audiocodec 下有 "Play" 和 "Record" 两个 verb

ALSA 官方默认定义有一些 verb 的名字, 如 "HiFi", "Voice", 但为了 Tina 平台下不同
方案的统一, **不推荐** 使用这些名字, 而应遵循后面的 verb 的命名约定

### device

表示设备, 如 "Headphone", "Speaker" 等, 每个 verb 需要包含一个或以上的 device,
其可用的命名请参考后面的 device 的命名约定

### modifier

表示修饰, 是对某个设备使用时附加的一些设置, 使同一个 verb 下的同一个 device 可以
有不同的使用效果, 如 "PlayTone", "PlayMusic" 等. modifier 并不是必须的, 每一个
verb 可以包含零至多个 modifier. 其可用的命名并无限制, 只要遵循通用命名约定
即可

## 语法

以 ucm_configs/DEMO/Default 目录下的配置文件为例

### 定义 card

新建一个目录, 并在该目录中新建一个与该目录同名的 .conf 文件, 如目录 *democard*
及其中的 *democard.conf* 就定义了一个名为 `democard` 的 card

*democard.conf* :
```
Comment "Comment on democard"

SectionUseCase."Play" {
    File "Play.conf"
}

ValueDefaults {
    Card "hw:demo"
    Foobar "value of Foobar in democard"
}
```

- 开头的 `Comment` 关键字是对这个 card 的注释
- `SectionUseCase` 关键字用于定义 verb, 上述例子中定义了名为 `Play` 的 verb, 其
详细内容位于文件 *Play.conf* 中
- `ValueDefaults` 关键字用于定义 card 级别的变量值, 上述例子中定义的 `Foobar` 和
`Card` 变量的有效作用域为该 card 及其以下的 verb, device 和 modifier. 可在 verb,
device 或 modifier 中定义同名变量来覆盖这些变量的值

### 定义 verb

如上所述, 在 democard 这个 card 的定义中, 通过 *democard.conf* 定义了名为 `Play`
的 verb, 此时需要创建 *Play.conf* 文件以描述 `Play` 的详细内容

*Play.conf* 中 verb 相关的部分:
```
SectionVerb {
    Commnent "Comment on verb Play"

    EnableSequence [
        cdev "hw:demo"

        cset "name='Play Switch' 1"

    ]

    DisableSequence [
        cdev "hw:foobar"

        cset "name='Play Switch' 0"
    ]

    Value {
        Foobar "Value of Foobar in verb Play"
    }
}
```

- `SectionVerb` 关键字中的就是 `Play` 这个 verb 的内容
- `Comment` 关键字是对这个 verb 的注释
- `EnableSequence` 指使能这个 verb 时的操作, `cdev` 指定控件对应的 ALSA 虚拟设备
名字, `cset` 指定需要操作的控件名字和它的值; `DisableSequence` 与 `EnableSequence`
类似, 指失能这个 verb 时的操作
(**注意** : 由于 ALSA UCM 自身的缺陷, `DisableSequence` 中的内容实际无法执行, 因此
**不推荐** 在 `SectionVerb` 中使用 `EnableSequence` 和 `DisableSequence` 关键字,
而应该把相关的使能和失能操作放到具体的 device 或 modifier 内容中; 另外,
`EnableSequence` 和 `DisableSequence` 使用的是 **方括号 [ ]** 而不是大括号 { })
- `Value` 关键字定义了 verb 级别的变量值, 其有效作用域为该 verb 以及其下的 device
和 modifier. 此处中的 `Foobar` 变量的值覆盖了 *democard.conf* 中 `ValueDefaults`
处定义的同名变量的值

### 定义 device

device 的定义也在 *Play.conf* 文件中, 通过 `SectionDevice` 关键字指定

*Play.conf* 中 device 相关的内容:
```
SectionDevice."Headphone" {
    Comment "Comment on device Headphone"

    EnableSequence [
        cdev "hw:demo"

        cset "name='Headphone Switch' 1"

    ]

    DisableSequence [
        cdev "hw:foobar"

        cset "name='Headphone Switch' 0"
    ]

    Value {
        Foobar "Value of Foobar in device Headphone"
    }
}
```

- `SectionDevice` 关键字用于定义 device, 此处定义了一个名为 `Headphone` 的 device
- `Comment`, `EnableSequence` 和 `DisableSequence` 关键字与 `SectionVerb` 中含义
相同 (不同之处在于 `SectionDevice` 中的 `EnableSequence` 和 `DisableSequence` 都
是可被实际执行的)
- `Value` 关键字定义 device 级别的变量值, 其有效作用域为该 device. 此处定义的
`Foobar` 变量的值覆盖了 `SectionVerb` 中同名变量的值

### 定义 modifier

除了使用的是 `SectionModifier` 关键字, modifier 的定义与 device 的类似. `Value`
关键字定义的变量值作用域为该 modifier, 会覆盖 `SectionVerb` 中同名变量的值

*Play.conf* 中 modifier 相关的内容:
```
SectionModifier."PlayTone" {
    Comment "Comment on modifier PlayTone"

    EnableSequence [
        cdev "hw:demo"

        cset "name='PlayTone Switch' 1"

    ]

    DisableSequence [
        cdev "hw:foobar"

        cset "name='PlayTone Switch' 0"
    ]

    Value {
        Foobar "Value of Foobar in modifier PlayTone"
    }
}
```

### 注释

UCM 配置文件中 `#` 符号后的内容会被认为是注释
``` conf
# This is a comment
SectionUseCase."Play" {
    File "Play.conf"
}
```

## 约定

为了保持 Tina 中不同方案下配置文件的统一, 有如下的约定:

### 通用命名约定

除了 card 和某些特定的 Value 的名字外, 其余自定义的名字均采用 **驼峰命名法**, 如
`LineIn`, `PlayTone`, `MicToHeadphone`, ... 名字中不包含空格, 下划线, 短横线等字
符

对于某些原本是全字母大写或部分字母大写的单词, 更倾向于将它们视作一个单词而仅
大写其首字母, 如: 使用 `Dmic` 而非 `DMIC`, `I2s` 而非 `I2S`, `AdcChannels` 而非
`ADCChannels`

### card

1. 全志 SoC 内置 codec 的名字统一使用 `audiocodec`
2. 内置 codec 以外的其他 card 的名字按照实际情况而定, 除了不能包含空格外并无额外
限制
3. 在 `ValueDefaults` 中需要定义变量 `Card` 指定对应的 ALSA 虚拟设备声卡名字
(不能使用数字编号, 而应该把声卡的名字写出来, 如 `hw:audiocodec`, `hw:sndcodec`)

### verb

1. verb 的命名遵循通用命名约定
2. 如无特殊情况, 推荐 verb 统一使用 `Play` , `Record` 和 `Call` 三个名字. `Play`
包含用于播放的设备, 如 `Headphone`, `Speaker` 等; `Record` 包含用于录
制的设备, 如 `Mic`, `LineIn` 等; `Call` 则包含兼有播放和录制功能的设备, 如
`Bluetooth`, `Headset` 等
3. **不推荐** 在 `SectionVerb` 中定义 `EnableSequence` 和 `DisableSequence`, 除非
只存在使能操作而不存在失能操作

### device

1. device 的命名遵循通用命名约定
2. 在一般情况下, 推荐使用以下的名字, 若无合适的再自行定义
> - Headphone
> - Headset
> - Speaker
> - Mic
> - LineIn
> - LineOut
> - PhoneIn
> - PhoneOut
> - Bluetooth
> - Spdif
> - Hdmi
3. 若某一个 verb 中存在多个同类型的设备, 可在 device 名字后面加编号区分 (编号不
一定需要从 1 开始, 可以根据实际情况而定), 然后在 `SectionVerb` 的 `Value` 中定义
`xxxNumber` 和 `xxxNames` 分别说明这种同类型设备的数量和名字, 名字之间用空格隔开
(如实际存在两个麦克风 MIC2 和 MIC3, 可以定义 `Mic2` 和 `Mic3` 两个 device, 并在
`SectionVerb` 的 `Value` 中定义 `MicNumber "2"` 和 `MicNames "Mic2 Mic3"`)
4. 有时候会有多个设备联合使用的情况, 对此没有硬性的命名约定, 对于某些情况推荐以下
命名:
> - 普通情况下多个设备联合使用, 使用 *序数词+设备* 的命名, 如 `TwoMics`, `FourMics`
> - 特定的设备需要对应特定的操作, 如 "MIC1 录制左声道, MIC2 录制右声道" 可命名为
> `Mic1LeftMic2Right`
> - 特定的设备数量与特定的通道数, 如两个麦克风可能分别用于录制两声道或四声道的不
> 同场景下, 可命名为 `TwoMicsTwoChs` 和 `TwoMicsFourChs` 以区分 (`Chs` 为 `Channels`
> 的缩写)
5. 若定义了多个设备联合使用的 device, 需要有它们各自本身的 device 定义. 例如定义
了 `TwoMics` 对应 MIC1 和 MIC2 的联合使用, 则需要分别定义 `Mic1` 和 `Mic2` 来提供
调节音量等操作的接口 (因为麦克风统一使用 `CaptureVolume` 变量作为音量控制的接口,
在当中不能同时定义多个设备的音量)

### modifier

除了需要遵循通用命名约定外, modifier 的命名由配置文件的编写者自行决定

### Value

推荐优先使用以下变量的命名, 若无合适的再自行定义 (自定义的名字应遵循通用命名约定)

- `Card` : 需要在 card 的 `ValueDefaults` 中定义, 用于指定该 card 对应的 ALSA 虚
拟声卡设备的名字 (不能使用数字编号, 而应该把具体的名字写出来), 如:
```
ValueDefaults {
    Card "hw:audiocodec"
}
```
- `PlaybackPCM` , `CapturePCM` : 在 verb, device 或 modifier 中用于指定具体的 PCM
设备, 如 `PlaybackPCM "hw:audiocodec,0"` (这两个变量的名字沿用了 ALSA 官方的名字,
因此没遵循本文档的通用命名约定)
- `PlaybackVolume` , `CaptureVolume` : 指定音量控件的名字及其默认值, 格式为: "控
件ID 默认值", 如 `PlaybackVolume "name='Headphone Volume' 40"`
- `PlaybackVolumeMin` , `PlaybackVolumeMax` , `CaptureVolumeMin` , `CaptureVolumeMax` :
指定音量数值的有效范围, 需要在定义了 `PlaybackVolume` 或 `CaptureVolume` 的同时
定义, 如:
```
Value {
    PlaybackVolume "name='Headphone Volume' 40"
    PlaybackVolumeMin "0"
    PlaybackVolumeMax "63"
}
```
- `PlaybackRate` , `CaptureRate` : 表明支持的特定采样率, 如
`PlaybackRate "16000 32000"` 表明仅支持 16000Hz 和 32000Hz 的采样率播放
- `PlaybackChannels` , `CaptureChannels` : 表明支持的特定通道数, 如
`PlaybackChannels "2 4"` 表明仅支持两通道或四通道播放
- `PlaybackFormat` , `CaptureFormat` : 表明支持的特定格式, 语法与 *aplay* 和
*arecord* 中 "-f" 选项的参数相同, 如 `PlaybackFormat "S16_LE S32_LE"` 表明仅支持
S16_LE 和 S32_LE 的格式播放

## 其他

### 顺序

(与 [interfaces.md](./interfaces.md) 中的 "默认使用案例 API" 相关联)

verb, device 和 modifier 的顺序是根据配置文件中的先后位置决定的, 因此在编写配置文
件时, 尽量将想要默认使用的项放在前面, 保证 alsa-ucm-aw 中的程序能优先选中它们

card 则是按照文件名的先后顺序排列, 除了修改名字外, 无法直接规定其顺序
