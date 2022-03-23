#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <string>
#include <iostream>
#include <fcntl.h>
#include <tconfigs/audio/common/pipeline.h>
#include <sys/time.h>


std::string config_file = "/etc/tutuclear/config.json";
std::shared_ptr<tconfigs::audio::Pipeline> capture_pipeline = nullptr;
std::shared_ptr<tconfigs::audio::Pipeline> background_music_pipeline = nullptr;
std::shared_ptr<tconfigs::audio::Pipeline> wakeup_hint_pipeline = nullptr;

unsigned int g_standby_count = 0;
unsigned int g_abnormal_wakeup_count= 0;
int standby_interval = 0;
int standby_delay = 0;
int capture_delay = 0;
int bgm_delay = 0;
bool has_bgm = true;
bool is_mad_test = false;
int keyword_detected_cnt = 0;
int capture_overrun_cnt = 0;
int bgm_underrun_cnt = 0;

void WhenSignal(int sig)
{
    switch (sig) {
    case SIGINT:
    case SIGQUIT:
    case SIGHUP:
        exit(-1);
        break;
    case SIGUSR1:
        if (has_bgm)
            background_music_pipeline->SetState(tconfigs::audio::State::kPaused);
        capture_pipeline->SetState(tconfigs::audio::State::kPaused);
        usleep(standby_delay);
        system("echo mem > /sys/power/state");
        capture_pipeline->SetState(tconfigs::audio::State::kPlaying);
        if (has_bgm)
            background_music_pipeline->SetState(tconfigs::audio::State::kPlaying);
        break;
    case SIGPIPE:
    default:
        break;
    }
}

void PrintHelpMsg(void)
{
    std::cout << "tutuclear-demo [(-c | --config) <config_file_path>]" << std::endl;
}

void mad_standby()
{
    char buf[32];
    int fd;
    const char *wakeup_count_node = "/sys/power/wakeup_count";
    const char *standby_node = "/sys/power/state";

    fd = open(wakeup_count_node, O_RDWR);
    if (fd < 0) {
        std::cout << "open wakeup_count failed" << std::endl;
        return ;
    }
    read(fd, buf, sizeof(buf));
    write(fd, buf, strlen(buf));
    close(fd);

    fd = open(standby_node, O_WRONLY);
    if (fd < 0) {
        std::cout << "open state failed" << std::endl;
        return ;
    }
    strcpy(buf, "mem");
    write(fd, buf, strlen(buf));
    close(fd);

    return ;
}

bool can_enter_mad_standby()
{
    int fd;
    char buf[128];
    int value = 0;

    fd = open("/sys/devices/platform/soc/mad/sunxi_mad_audio/lpsd_status", O_RDONLY);
    if (fd < 0) {
        std::cout << "open lpsd_status failed" << std::endl;
        return true;
    }
    memset(buf, 0, sizeof(buf));
    read(fd, buf, sizeof(buf));
    printf("lpsd_status:%s\n", buf);
    value = atoi(buf);
    close(fd);
    if (value == 0)
        return true;
    return false;
}

int main(int argc, char *argv[])
{
    signal(SIGHUP, WhenSignal);
    signal(SIGQUIT, WhenSignal);
    signal(SIGINT, WhenSignal);
    signal(SIGPIPE, WhenSignal);
    signal(SIGUSR1, WhenSignal);

    const struct option long_opts[] = {
        {"help", no_argument, NULL, 'h'},
        {"config", required_argument, NULL, 'c'},
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "hc:", long_opts, NULL)) != -1) {
        switch (opt) {
        case 'h' :
            PrintHelpMsg();
            return 0;
        case 'c':
            config_file = optarg;
            break;
        default:
            std::cout << "Invalid option" << std::endl;
            PrintHelpMsg();
            return -1;
        }
    }

    std::cout << "Config file: " << config_file << std::endl;


    {
        // The following objects will be destructed automatically after going
        // out of this scope

        auto config_json = tconfigs::json::JsonStringFromFile::Create(config_file);
        if (!config_json) {
            std::cout << "Fail to get JSON from config file" << std::endl;
            return -1;
        }

        rapidjson::Document doc;
        if (!tconfigs::json::ParseJsonToDomRelaxed(config_json->string(), &doc)) {
            std::cout << "Fail to parse JSON to DOM" << std::endl;
            return -1;
        }

        const rapidjson::Value* config = nullptr;
        if (!tconfigs::json::pointer::GetObject(doc, "/capture_pipeline", &config)) {
            std::cout << "Fail to get config \"capture_pipeline\"" << std::endl;
            return -1;
        }
        if (!tconfigs::json::pointer::GetInt(*config, "/standby_interval", &standby_interval)) {
            standby_interval = 5;
        }
        if (!tconfigs::json::pointer::GetInt(*config, "/standby_delay", &standby_delay)) {
            standby_delay = 500000;
        }
        if (!tconfigs::json::pointer::GetBool(*config, "/has_bgm", &has_bgm)) {
            has_bgm = true;
        }
        if (!tconfigs::json::pointer::GetInt(*config, "/capture_delay", &capture_delay)) {
            capture_delay = 0;
        }
        if (!tconfigs::json::pointer::GetInt(*config, "/bgm_delay", &bgm_delay)) {
            bgm_delay = 0;
        }
        if (!tconfigs::json::pointer::GetBool(*config, "/is_mad_test", &is_mad_test)) {
            is_mad_test = false;
        }
        capture_pipeline = tconfigs::audio::Pipeline::Create("capture_pipeline", *config);
        if (!capture_pipeline) {
            std::cout << "Fail to create capture_pipeline" << std::endl;
            return -1;
        }

        if (!tconfigs::json::pointer::GetObject(doc, "/background_music_pipeline", &config)) {
            std::cout << "Fail to get config \"background_music_pipeline\"" << std::endl;
            return -1;
        }
        background_music_pipeline = tconfigs::audio::Pipeline::Create(
                "background_music_pipeline", *config);
        if (!background_music_pipeline) {
            std::cout << "Fail to create background_music_pipeline" << std::endl;
            return -1;
        }

        if (!tconfigs::json::pointer::GetObject(doc, "/wakeup_hint_pipeline", &config)) {
            std::cout << "Fail to get config \"wakeup_hint_pipeline\"" << std::endl;
            return -1;
        }
        wakeup_hint_pipeline = tconfigs::audio::Pipeline::Create(
                "wakeup_hint_pipeline", *config);
        if (!wakeup_hint_pipeline) {
            std::cout << "Fail to create wakeup_hint_pipeline" << std::endl;
            return -1;
        }
    }

    capture_pipeline->AddExternalMessageCallback("TutuclearKwd", [&] {
        ++keyword_detected_cnt;
        std::cout << "Keyword detected count: " << keyword_detected_cnt << std::endl;
        wakeup_hint_pipeline->SetState(tconfigs::audio::State::kPlaying);
    });

    capture_pipeline->AddExternalMessageCallback("CaptureOverrun", [&] {
        ++capture_overrun_cnt;
        std::cout << "Capture overrun count: " << capture_overrun_cnt << std::endl;
    });

    background_music_pipeline->AddExternalMessageCallback("BgmUnderrun", [&] {
        ++bgm_underrun_cnt;
        std::cout << "Background music underrun count: " << bgm_underrun_cnt << std::endl;
    });

    capture_pipeline->SetState(tconfigs::audio::State::kPlaying);
    if (has_bgm)
        background_music_pipeline->SetState(tconfigs::audio::State::kPlaying);
    if (is_mad_test) {
        while(1) {
            sleep(standby_interval);
            if (can_enter_mad_standby()) {
                struct timeval tv_before, tv_after, tv_sub;

                if (has_bgm)
                    background_music_pipeline->SetState(tconfigs::audio::State::kPaused);
                if (capture_delay != 0)
                    sleep(capture_delay);
                capture_pipeline->SetState(tconfigs::audio::State::kPaused);
                usleep(standby_delay);
                g_standby_count++;
                std::cout << "==================================" << std::endl;
                std::cout << "standby count: " << g_standby_count << std::endl;
                std::cout << "abnormal wakeup count: " << g_abnormal_wakeup_count << std::endl;
                std::cout << "==================================" << std::endl;
                gettimeofday(&tv_before, NULL);
                mad_standby();
                gettimeofday(&tv_after, NULL);
                timersub(&tv_after, &tv_before, &tv_sub);
                printf("sleep time:%umsec\n", tv_sub.tv_sec*1000+tv_sub.tv_usec/1000);
                if (tv_sub.tv_sec == 0 && tv_sub.tv_usec < 600000)
                    g_abnormal_wakeup_count++;
                capture_pipeline->SetState(tconfigs::audio::State::kPlaying);
                std::cout << "resume from standby!" << std::endl;
                usleep(10000);
                if (has_bgm) {
                    if (bgm_delay != 0)
                        sleep(bgm_delay);
                    background_music_pipeline->SetState(tconfigs::audio::State::kPlaying);
                }
            }
        }
    }

    // The TUTUClear library will expire after continuously running 4320 minutes.
    sleep(60 * 4320);
    std::cout << "tutuclear-demo has expired. Please rerun it." << std::endl;

    return 0;
}
