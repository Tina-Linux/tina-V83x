# Interfaces

此文档介绍了 alsa-ucm-aw-lib 提供的 C 语言 API

(前缀 "aua" 是 "ALSA UCM AW" 的首字母缩写)

## 使用方法

在应用程序代码中包含头文件 `alsa-ucm-aw.h` , 在编译时加上链接参数 `-lalsa-ucm-aw`

## API

### 通用 API

#### aua_use_case_enable, aua_use_case_disable
```c
/**
 * Run the EnableSequence/DisableSequence of a use case
 * @param card_name:    sound card name
 * @param verb_name:    verb name
 * @param dev_name:     device name
 * @param mod_name:     modifier name
 * @return: 0 on success, otherwise on error
 */
int aua_use_case_enable(const char *card_name, const char *verb_name,
                        const char *dev_name, const char *mod_name);
int aua_use_case_disable(const char *card_name, const char *verb_name,
                         const char *dev_name, const char *mod_name);
```

用于执行配置文件中的某个 `EnableSequence` / `DisableSequence`

参数中的 `card_name` 和 `verb_name` 不能为 `NULL` , `dev_name` 和 `mod_name` 中
至少有一个不为 `NULL`

#### aua_value_get
```c
/**
 * Get the "Value" defined in UCM configurations
 * @param value_name:       value name (e.g. PlaybackPCM, PlaybackVolume, CapturePCM, etc.)
 * @param card_name:        sound card name
 * @param verb_name:        verb name
 * @param dev_or_mod_name:  device or modifier name
 * @return: the string of "Value" on success; NULL on errer
 *
 * Note:
 *      The returned string is dynamically allocated. It needs be deallocated
 *      with free().
 */
char *aua_value_get(const char *value_name, const char *card_name,
                    const char *verb_name, const char *dev_or_mod_name);
```

用于获取 UCM 配置文件中的某个 *Value* 的值

参数中的 `card_name` , `verb_name` 和 `dev_or_mod_name` 可为 `NULL` , 它们用于决
定 *Value* 所在的位置

#### aua_playback_volume_get, aua_capture_volume_get
```c
/**
 * Get the volume according to the ALSA control defined in "PlaybackVolume" /
 * "CaptureVolume" in UCM configurations
 * @param card_name:        (in) sound card name
 * @param verb_name:        (in) verb name
 * @param dev_or_mod_name:  (in) device or modifier name
 * @param volume:           (out) the volume value got
 * @return: 0 on success, otherwise on error
 */
int aua_playback_volume_get(const char *card_name, const char *verb_name,
                            const char *dev_or_mod_name, int *volume);
int aua_capture_volume_get(const char *card_name, const char *verb_name,
                           const char *dev_or_mod_name, int *volume);
```

根据 `PlaybackVolume` 或 `CaptureVolume` 中 ALSA 控件的名字获取对应控件的值

参数 `card_name` , `verb_name` , `dev_or_mod_name` 的使用与 `aua_value_get` 的相同

#### aua_playback_volume_set, aua_capture_volume_set
```c
/**
 * Set the volume according to the ALSA control defined in "PlaybackVolume" /
 * "CaptureVolume" in UCM configurations
 * @param card_name:        sound card name
 * @param verb_name:        verb name
 * @param dev_or_mod_name:  device or modifier name
 * @param volume:           the volume value to set
 * @return: 0 on success, otherwise on error
 */
int aua_playback_volume_set(const char *card_name, const char *verb_name,
                            const char *dev_or_mod_name, int volume);
int aua_capture_volume_set(const char *card_name, const char *verb_name,
                           const char *dev_or_mod_name, int volume);
```

与 `aua_playback_volume_get` 和 `aua_capture_volume_get` 相反的函数, 根据
`PlaybackVolume` 或 `CaptureVolume` 中 ALSA 控件的名字设置该控件的值

#### aua_playback_volume_set_default, aua_capture_volume_set_default
```c
/**
 * Set the volume according to the ALSA control and its default value defined in
 * "PlaybackVolume"/"CaptureVolume" in UCM configurations
 * @param card_name:        sound card name
 * @param verb_name:        verb name
 * @param dev_or_mod_name:  device or modifier name
 * @return: 0 on success, otherwise on error
 */
int aua_playback_volume_set_default(const char *card_name, const char *verb_name,
                                    const char *dev_or_mod_name);
int aua_capture_volume_set_default(const char *card_name, const char *verb_name,
                                   const char *dev_or_mod_name);
```

类似于 `aua_playback_volume_set` 和 `aua_capture_volume_set` , 不过会将 ALSA 控件
的值设置为 `PlaybackVolume` 或 `CaptureVolume` 中指定的默认值

#### aua_playback_volume_min_get, aua_playback_volume_max_get, aua_capture_volume_min_get, aua_capture_volume_max_get
```c
/**
 * Get the min/max volume according to "PlaybackVolumeMin"/"PlaybackVolumeMax"/
 * "CaptureVolumeMin"/"CaptureVolumeMax" in UCM configurations
 * @param card_name:        sound card name
 * @param verb_name:        verb name
 * @param dev_or_mod_name:  device or modifier name
 * @param volume:           (out) the volume value got
 * @return: 0 on success, otherwise on error
 */
int aua_playback_volume_min_get(const char *card_name, const char *verb_name,
                                const char *dev_or_mod_name, int *volume);
int aua_playback_volume_max_get(const char *card_name, const char *verb_name,
                                const char *dev_or_mod_name, int *volume);
int aua_capture_volume_min_get(const char *card_name, const char *verb_name,
                               const char *dev_or_mod_name, int *volume);
int aua_capture_volume_max_get(const char *card_name, const char *verb_name,
                               const char *dev_or_mod_name, int *volume);
```

获取 `PlaybackVolumeMin` 或 `PlaybackVolumeMax` 或 `CaptureVolumeMin` 或
`CaptureVolumeMax` 中的数值

#### 例子

假设 card *audiocodec* 的 verb *Play* 中有如下的 device *Headphone* 的定义:
```
SectionDevice."Headphone" {
    EnableSequence [
        cdev "hw:demo"
        cset "name='Headphone Switch' 1"
    ]

    DisableSequence [
        cdev "hw:foobar"
        cset "name='Headphone Switch' 0"
    ]

    Value {
        PlaybackVolume "name='Headphone Volume' 40"
        PlaybackVolumeMin "0"
        PlaybackVolumeMax "63"
    }
}
```

当前系统中控件 name='Headphone Volume' 的实际值为 35

则上述 API 的使用方法如下 (忽略了错误返回值的判断)
```c
/* 执行 EnableSequence 的内容 */
err = aua_use_case_enable("audiocodec", "Play", "Headphone", NULL);

/* 字符串 str 的值为 "name='Headphone Volume' 40" (使用完后需释放 str: free(str))*/
str = aua_value_get("PlaybackVolume", "audiocodec", "Play", "Headphone");

/* 返回成功后变量 vol 的值为 35*/
err = aua_playback_volume_get("audiocodec", "Play", "Headphone", &vol);

/* 返回成功后系统中控件 name='Headphone Volume' 的值为 45 */
err = aua_playback_volume_set("audiocodec", "Play", "Headphone", 45);

/* 返回成功后系统中控件 name='Headphone Volume' 的值为 40 */
err = aua_playback_volume_set_default("audiocodec", "Play", "Headphone");

/* 返回成功后变量 vol_min 的值为 0, vol_max 的值为 63 */
err = aua_playback_volume_min_get("audiocodec", "Play", "Headphone", &vol_min);
err = aua_playback_volume_max_get("audiocodec", "Play", "Headphone", &vol_max);
```

### 默认使用案例 API

函数名均以 `aua_default_play_` 和 `aua_default_record_` 为前缀, 使用方法与通用
API 类似, 但它们的参数 `card_name` , `verb_name` , `dev_name` , `mod_name` 允许为
`NULL` , 此时它们会自动在当前系统的配置文件中按照以下策略寻找默认的 card, verb,
device 或 modifier:

1. 当 `card_name` 为 `NULL` 时, 优先选择 `audiocodec` 作为默认的 card, 若不存在
`audiocodec` , 则选择 card 列表中的第一项
2. 当 `verb_name` 为 `NULL` 时, 前缀 `aua_default_play_` 的函数会优先选择 `Play`
为默认的 verb (前缀 `aua_default_capture_` 的函数则会优先选择 `Record` ), 若不存
在, 则选择 verb 列表中的第一项
3. 当 `dev_name` 和 `mod_name` 均为 `NULL` 时, 才会去寻找默认的 device 或 modifier,
这两者没有优先选择的名字, 会直接选择列表中的第一项

若这些参数不为 `NULL` , 则函数的使用效果与通用 API 中的相同

从应用程序的角度而言, 若没有特殊的需求, 可以将全部的这些参数设为 `NULL` , 让其自
动根据当前系统的配置文件选择合适设备, 保证了代码的可移植性
