# README

ALSA UCM (Use Case Manager) 通过文本文件记录了不同使用场景下 ALSA 控件 (control)
的配置, 这些配置文件位于 /usr/share/alsa/ucm 目录下

## alsa-ucm-aw

alsa-ucm-aw 包含了 alsa-ucm-aw-configs, alsa-ucm-aw-lib, alsa-ucm-aw-utils 这几
个软件包, 其功能分别如下:

#### alsa-ucm-aw-configs

提供了 Tina 中不同方案 (平台) 下的 ALSA UCM 配置文件, 详情请参考
[ucm_config.md](./ucm_config.md)

#### alsa-ucm-aw-lib

提供了 C 语言 API 以供其他应用程序使用 ALSA UCM 配置文件, 详情请参考
[interfaces.md](./interfaces.md)

#### alsa-ucm-aw-utils

提供了应用程序 alsa-ucm-aw 以使用 Tina 中 ALSA UCM 配置文件, 详情请参考
[utils.md](./utils.md) (ALSA 原生的 alsa-utils 中也提供了使用 UCM 配置文件的应用
程序 alsaucm, 但目前其功能有不少缺陷, 也缺少说明文档, 因此不推荐使用)

## 参考资料

ALSA UCM 在网上的资料并不多, 也缺少完整描述其语法和使用方法的资料, 以下为设计
alsa-ucm-aw 过程中参考过的一些资料, 供感兴趣者参考:
- [Use Case Interface](http://www.alsa-project.org/alsa-doc/alsa-lib/group__ucm.html)
- [Help me obtain complete document of alsa ucm](http://mailman.alsa-project.org/pipermail/alsa-devel/2013-November/068763.html)
- [alsaucm should come with a man page](http://mailman.alsa-project.org/pipermail/alsa-devel/2016-December/115382.html)
- [ALSA UCM](http://mailman.alsa-project.org/pipermail/alsa-devel/2010-November/033359.html)
- [Using UCM with PulseAudio](http://mailman.alsa-project.org/pipermail/alsa-devel/2012-June/052936.html)
- [UCM list/set/get](http://mailman.alsa-project.org/pipermail/alsa-devel/2011-January/036280.html)

