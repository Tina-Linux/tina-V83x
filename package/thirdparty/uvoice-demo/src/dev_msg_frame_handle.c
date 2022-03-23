#include "dev_msg_frame_handle.h"

struct timespec ts;
long reply_timeout = 0;

int get_device_match_id(uint8_t roomtype, uint8_t devicetype, uint8_t* dev_id)
{
    int ret = -1;
    FILE *fp;
    int dev_num = NUMS_OF_DEV_CONFIGS;

    device_configs_t* frame = (device_configs_t*)malloc(sizeof(device_configs_t) \
            + dev_num * 8 * sizeof(char));
    memset(frame, 0, sizeof(device_configs_t) + dev_num * 8 * sizeof(char));
    if (frame == NULL) {
        LOGE("frame malloc fail!");
        goto fail;
    }

    fp = fopen(FILE_OF_DEVICE_INFO,"r+");
    if(fp == NULL)
    {
        LOGE("open/create file: %s fail", FILE_OF_DEVICE_INFO);
        goto fail;
    } else {
        LOGD("open/create file: %s success", FILE_OF_DEVICE_INFO);
    }

    for (int i = 0; i < dev_num; i++) {
        fread(frame, sizeof(device_configs_t) + dev_num * 8 * sizeof(char), 1, fp);

        for (int j = 0; j < 8; j++) {
            printf("%02X ", frame->info[i * 8 + j]);
        }
        printf("\n");

    }

    for (int i = 0; i < dev_num; i++) {
        if ((frame->info[i * 8] == roomtype) && (frame->info[i * 8 + 1] == devicetype)){
            memcpy(dev_id, &frame->info[i * 8 + 2], 6 * sizeof(char));

            printf("FOUND DEV: [%02X] [%02X] ", roomtype, devicetype);
            for (int j = 0; j < 6; j++) {
                printf("%02X ", dev_id[j]);
            }
            printf("\n\n");

            ret = 0;
            break;
        } else if (((frame->info[i * 8] != roomtype) || (frame->info[i * 8 + 1] != devicetype)) &&    \
            (i == dev_num - 1)) {
            ret = -1;
        }
    }
fail:
    if (fp != NULL) {
        fclose(fp);
    }
    if (frame != NULL) {
        free(frame);
    }
    return ret;
}

static void *process_frame_thread(void *arg)
{
    int i, ret;
    uint8_t buffer[ASR_UART_BUFFER_SIZE];
    //uint8_t buffer[ASR_UART_BUFFER_SIZE] = {0xAA,0xD4,0x17,0x00,0x00,\
		0xA2,0x01,0x02,0x02,0x28,0x0A,0x6C,0x07,0x10,\
		0x10,0x01,0x01,0x00,0x03,0x00,0x63,0x00,0x69};//add for debug by zhouhuo@20190403
    //uint8_t buffer[ASR_UART_BUFFER_SIZE] = {0xAA,0xD4,0x58,0x00,0x00,0xA0,0x01,\
		0x00,0xFA,0x34,0x5B,0xBB,0x34,0xA5,0x1B,\
		0x00,0xFD,0x34,0x5B,0xBB,0x34,0xA5,0x1A,\
		0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,\
		0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,\
		0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,\
		0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,\
		0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,\
		0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,\
		0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,\
		0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,\
		0xCC};
		//add for debug eii devs configs by zhouhuo@20190409
    uint8_t len;
    //struct msg_buf msgbuf;

    while(1)
    {
/*
        LOCK_MSG_QUEUE();
        ret = recv_msg_from_queue(gs_msgid, &msgbuf);
        if (flag ==  1) {
            memcpy(&msgbuf, msgbuf.data, msgbuf.dataLen);
        }
        UNLOCK_MSG_QUEUE();
        if (ret == RESULT_FAIL) {

        printf("read msg from queue and process frame\n");
        printf("recv msgbuf.dataLen = %d\n",msgbuf.dataLen);
        uart_debug_hex_dump("process frame = [", msgbuf.data, msgbuf.dataLen);
*/
        if (msgbuf.update_flag != TRUE) {
            usleep(10 * 1000);
            continue;
        }

        msgbuf.update_flag = FALSE;
        memcpy(buffer, msgbuf.data, msgbuf.dataLen); //can for debug
        len = msgbuf.dataLen;

        cmd_uart_msg_t* frame = (cmd_uart_msg_t*)buffer;

#if DEBUG_ASR_UART
        printf("asr recv:[%02X]\n", frame->msgId);
        for (i = 0; i < frame->msgLen; ++i) {
            printf("%02X ", ((uint8_t*)frame)[i]);
        }
        printf("\n");
#endif

        int ret = ASR_AC_ERROR_NONE;
        /*process predefined callback first*/
        for (i = 0; i < PROCESSORS_COUNT; ++i) {
            if (processors[i].cmd == frame->msgId && processors[i].processor != 0) {
                ret = processors[i].processor(frame);
                m_ac.err = ret;
                break;
            }
        }
/*
        if (frame->msgId != ASR_UART_MSG_HEART_BEAT
                && frame->msgId != ASR_UART_MSG_QUERY_AC_STATUS) {
            m_ac.play = frame->play;
        }
*/
        /*then push result to user layer*/
        /*conflict with device_asr_send_frame_with_reply*/
        /*LOCK_UART()*/

        if (m_asr_uart.write_request != NULL) {
            if (m_asr_uart.write_request->msgId == frame->msgId) {
                if (m_asr_uart.reply != NULL) {
                    free(m_asr_uart.reply);
                    m_asr_uart.reply = NULL;
                }

                if (m_asr_uart.requst_need_reply) {
                    m_asr_uart.reply = (cmd_uart_msg_t*)malloc(len);
                    if (!m_asr_uart.reply) {
                        log_e("mem alloc fail");
                        return;
                    }
                    memcpy(m_asr_uart.reply, buffer, len);
                }
                sem_post(&m_asr_uart.write_sem);
                /*pthread_cond_signal(&m_asr_uart.write_signal);*/
            }
        }

        /*UNLOCK_UART()*/
    }
    pthread_exit("process_frame_thread exit\n");
}


static void reset_recv()
{
    recv_controller.state = ASR_UART_RECV_HEADER;
}


static void *reader_thread(void *arg)
{
    char  ch;
    int ret, i = 0;
    int64_t last_read_milisec = 0;
    int64_t milisec;
    struct timeval value;
    gettimeofday(&value,NULL);
    m_asr_uart.last_heartbeat_ticks = (uint64_t)((value.tv_sec*1000 + value.tv_usec/1000) - ASR_UART_HEARTBEAT_INTERVAL);

    LOGD("start ...");
    while (1) {
        ret = app_uart_read(device_asr_uart_fd, &ch, 1, READ_TIMEOUT_MS);
        gettimeofday(&value, NULL);
        milisec = (uint64_t)(value.tv_sec * 1000 + value.tv_usec / 1000);

        if (ret == 1) {
            //if (milisec - last_read_milisec > 50) { //delete for eii devs configs reset recv by zhouhuo@20190409
            if (milisec - last_read_milisec > 100) {
                reset_recv();
                last_read_milisec = milisec;
                LOGD("reset_recv...");
            }

            /*update last active ticks when any char recved*/
            m_asr_uart.last_active_ticks = milisec;
            //LOGT("m_asr_uart.last_active_ticks = %llu", m_asr_uart.last_active_ticks);
            switch (recv_controller.state) {
            case ASR_UART_RECV_HEADER:
                if (ch == ASR_FRAME_HEADER) {
                    recv_controller.state = ASR_UART_RECV_ID;
                    recv_controller.recvIndex = 0;
                    recv_controller.dataLen = 0;
                    recv_controller.recvBuffer[recv_controller.recvIndex++] = ch;
                    recv_controller.sum = ch;
                }
                break;
            case ASR_UART_RECV_ID:
                recv_controller.msgId = ch;
                recv_controller.recvBuffer[recv_controller.recvIndex++] = ch;
                recv_controller.state = ASR_UART_RECV_LEN;
                recv_controller.sum += ch;
                LOGT("recv msg id = %#x", ch);
                break;
            case ASR_UART_RECV_LEN:
                recv_controller.recvBuffer[recv_controller.recvIndex++] = ch;
                recv_controller.dataLen = ch;
                recv_controller.state = ASR_UART_RECV_SPEAKER_SWITCH;
                recv_controller.sum += ch;
                LOGT("recv msg len = %d, max len = %d", ch, ASR_UART_BUFFER_SIZE);
                if (ch + 6 >= ASR_UART_BUFFER_SIZE) {
                    LOGW("recv len exceeded");
                    reset_recv();
                }
                break;
            case ASR_UART_RECV_SPEAKER_SWITCH:
                recv_controller.recvBuffer[recv_controller.recvIndex++] = ch;
                recv_controller.state = ASR_UART_RECV_RESVED;
                recv_controller.sum += ch;
                break;
            case ASR_UART_RECV_RESVED:
                recv_controller.recvBuffer[recv_controller.recvIndex++] = ch;
                recv_controller.state = ASR_UART_RECV_DATA;
                recv_controller.sum += ch;
                break;
            case ASR_UART_RECV_DATA:
                recv_controller.recvBuffer[recv_controller.recvIndex++] = ch;
                recv_controller.sum += ch;
                if (recv_controller.recvIndex == recv_controller.dataLen - 1) {
                    recv_controller.state = ASR_UART_RECV_SUM;
                }
                break;
            case ASR_UART_RECV_SUM:
                recv_controller.recvBuffer[recv_controller.recvIndex++] = ch;
                if (recv_controller.sum == ch) {
                    //LOCK_MSG_QUEUE();
                    memcpy(msgbuf.data, recv_controller.recvBuffer, recv_controller.dataLen);
                    msgbuf.dataLen = recv_controller.dataLen;
                    msgbuf.mtype = getpid();
                    //uart_debug_hex_dump("msgbuf = [", msgbuf.data, msgbuf.dataLen);
                    //printf("msgbuf.dataLen = %d\n",msgbuf.dataLen);
                    msgbuf.update_flag = TRUE;
                    //send_msg_to_queue(gs_msgid, &msgbuf);
                    //UNLOCK_MSG_QUEUE()
                } else {
                    reply_timeout = SUM_ERR_MS;
                    LOGW("sum error,%d,%d", recv_controller.sum, ch);
                }
            default:
                reset_recv();
                break;
            }
        }
        else {
            if (milisec - m_asr_uart.last_heartbeat_ticks >= ASR_UART_HEARTBEAT_INTERVAL) {
                if (!m_asr_uart.not_get_last_requst_reply) {
                    if (send_heartbeat() == ASR_AC_ERROR_NONE) {
                        m_asr_uart.last_heartbeat_ticks = milisec;
                    } else {
                        /*try next READ_TIMEOUT_MS*/
                        m_asr_uart.last_heartbeat_ticks = milisec - ASR_UART_HEARTBEAT_INTERVAL + READ_TIMEOUT_MS;
                    }
                } else {
                    /*try next READ_TIMEOUT_MS*/
                    continue;
                }
            }
            /*else if (ticks - m_asr_uart.last_query_ticks >= ASR_UART_STATUS_QUERY_INTERVAL)
            if (milisec - m_asr_uart.last_query_ticks >= ASR_UART_STATUS_QUERY_INTERVAL) {
                if (query_ac_status() == ASR_AC_ERROR_NONE) {
                    m_asr_uart.last_query_ticks = milisec;
                } else {
                    m_asr_uart.last_query_ticks = milisec - ASR_UART_HEARTBEAT_INTERVAL + 500;
                }
            }*/
        }
    }
    pthread_exit(NULL);
    LOGD("end ...");
    return NULL;
}

int device_asr_reader_init(void)
{
    uint8_t volume_buffer = 0xFF;
    memset(&recv_controller, 0, sizeof(recv_controller));
    reset_recv();
    m_asr_uart.readThread_PrivInitOk=0;

    memset(&timer_start, 0 , sizeof(struct timeval));
    memset(&timer_end, 0 , sizeof(struct timeval));
    gettimeofday(&timer_start, NULL);
    gettimeofday(&timer_end, NULL);
/*
    memset(&timer_msg_start, 0 , sizeof(struct timeval));
    memset(&timer_msg_end, 0 , sizeof(struct timeval));
    gettimeofday(&timer_msg_start, NULL);
    gettimeofday(&timer_msg_end, NULL);
*/
    //默认掉电上电开启识别和播报，即不保存配置
/*
    if (is_file_exist(FILE_OF_LOCAL_ASR_INFO, 10) == 0) {
        m_ac.local_dev_set_asr = ASR_ENABLE;
        LOGI("local ASR enable: %s", FILE_OF_LOCAL_ASR_INFO);
    } else {
        m_ac.local_dev_set_asr = ASR_DISABLE;
        LOGI("local ASR disable: %s", FILE_OF_LOCAL_ASR_INFO);
    }

    if (is_file_exist(FILE_OF_LOCAL_PLAYBACK_INFO, 10) == 0) {
        m_ac.local_dev_set_play = ASR_ENABLE;
        LOGI("local playback enable: %s", FILE_OF_LOCAL_PLAYBACK_INFO);
    } else {
        m_ac.local_dev_set_play = ASR_DISABLE;
        LOGI("local playback disable: %s", FILE_OF_LOCAL_PLAYBACK_INFO);
    }

    if (is_file_exist(FILE_OF_EII_ASR_INFO, 10) == 0) {
        m_ac.eii_dev_set_asr = ASR_ENABLE;
        LOGI("eii asr enable: %s", FILE_OF_EII_ASR_INFO);
    } else {
        m_ac.eii_dev_set_asr = ASR_DISABLE;
        LOGI("eii asr disable: %s", FILE_OF_EII_ASR_INFO);
    }

    if (is_file_exist(FILE_OF_EII_PLAYBACK_INFO, 10) == 0) {
        m_ac.eii_dev_set_play = ASR_ENABLE;
        LOGI("eii asr enable: %s", FILE_OF_EII_PLAYBACK_INFO);
    } else {
        m_ac.eii_dev_set_play = ASR_DISABLE;
        LOGI("eii asr disable: %s", FILE_OF_EII_PLAYBACK_INFO);
    }
*/
    m_ac.local_dev_set_asr = ASR_ENABLE;
    m_ac.local_dev_set_play = ASR_ENABLE;
    m_ac.eii_dev_set_asr = ASR_ENABLE;
    m_ac.eii_dev_set_play = ASR_ENABLE;
    volume_buffer = read_conf_info(FILE_OF_VOLUME_INFO);
/*
    uint8_t temp[10];
    LOGI("raw temp = %s",temp);
    sprintf(temp,"%d", volume_buffer);
    LOGI("temp = %s",temp);
    save_conf_info(FILE_OF_VOLUME_INFO, temp);
*/
    if ((volume_buffer !=  RESULT_ERR) && (volume_buffer > 0) && (volume_buffer <= 100)) {
        m_ac.volume = volume_buffer;
    } else {
        m_ac.volume = DEFAULT_VOLUME;
    }
    LOGD("volume_buffer = %d, m_ac.volume = %d", volume_buffer, m_ac.volume);

    if ((m_ac.volume > 0) && (m_ac.volume <= 20)) {
        system("amixer cset name='head phone volume' 46");
    } else if ((m_ac.volume > 20) && (m_ac.volume <= 40)) {
        system("amixer cset name='head phone volume' 49");
    } else if ((m_ac.volume > 40) && (m_ac.volume <= 60)) {
        system("amixer cset name='head phone volume' 53");
    } else if ((m_ac.volume > 60) && (m_ac.volume <= 80)) {
        system("amixer cset name='head phone volume' 55");
    } else if ((m_ac.volume > 80) && (m_ac.volume <= 100)) {
        system("amixer cset name='head phone volume' 59");
    }
    return 0;
}

void device_asr_reader_start()
{
    int ret = -1;

    if (m_asr_uart.readThread_PrivInitOk == 0) {
        LOGD("start...");

        if (pthread_mutex_init(&m_asr_uart.lock, NULL) != 0) {
            LOGE("pthread_mutex_init m_asr_uart.lock error");
            return;
        }

        if (pthread_mutex_init(&m_asr_uart.msg_queue_lock, NULL) != 0) {
            LOGE("pthread_mutex_init m_asr_uart.msg_queue_lock error");
            return;
        }

        #if 0
        if (pthread_cond_init(&m_asr_uart.write_signal,NULL) != 0) {
            log_e("pthread_cond_init m_asr_uart.write_signal error !\n");
            goto err_delete_lock;
        }
        #endif

        sem_init(&m_asr_uart.write_sem, 0, 0);
        m_asr_uart.write_request = NULL;
        m_asr_uart.requst_need_reply = 0;
        m_asr_uart.not_get_last_requst_reply = 0;
        m_asr_uart.reply = NULL;
        m_asr_uart.last_active_ticks = 0;
        m_asr_uart.last_heartbeat_ticks = 0;
        m_asr_uart.last_query_ticks = 0;
        device_asr_uart_fd = app_uart_open(device_asr_uart_fd, APP_UART);
        assert(device_asr_uart_fd);
        app_uart_Init(device_asr_uart_fd, ASR_UART_SET_SPEED, ASR_UART_SET_FLOWCTL, \
		ASR_UART_SET_DATABITS, ASR_UART_SET_STOPBITS, ASR_UART_SET_PARITY);

        memset(&msgbuf, 0, sizeof(struct msg_buf));

        ret = pthread_create(&m_asr_uart.readThread, NULL, reader_thread, NULL);
        if (ret != 0)
        {
            LOGE("reader_thread create error(%d), FIXME!", ret);
            goto err_delete_write_signal;
        }

        ret = pthread_create(&m_asr_uart.processframeThread, NULL, process_frame_thread, NULL);
        if (ret != 0)
        {
            LOGE("process_frame_thread create error(%d), FIXME!", ret);
            goto err_delete_write_signal;
        }

        LOGD("end...");
        m_asr_uart.readThread_PrivInitOk=1;
        return;

err_delete_write_signal:
        pthread_cond_destroy(&m_asr_uart.write_signal);
/*
err_delete_lock:
        pthread_mutex_destroy(&m_asr_uart.lock);
*/
    }
}

cmd_uart_msg_t* device_asr_alloc_frame(uint8_t msgId, uint8_t* data, uint8_t len)
{
    cmd_uart_msg_t* frame = (cmd_uart_msg_t*)malloc(sizeof(cmd_uart_msg_t) + len + 1);
    if (frame == NULL)
        return NULL;
    frame->header = ASR_FRAME_HEADER;
    frame->msgId = msgId;
    frame->msgLen = sizeof(cmd_uart_msg_t) + len + 1;
    frame->play = 0;
    frame->rsvd1 = 0;
    memcpy(frame->data, data, len);
    return frame;
}

cmd_uart_msg_t* device_asr_alloc_frame_ack_board(uint8_t msgId, uint8_t* data, uint8_t len, uint8_t ret_status)
{
    cmd_uart_msg_t* frame = (cmd_uart_msg_t*)malloc(sizeof(cmd_uart_msg_t) + len + 1);
    if (frame == NULL)
        return NULL;
    frame->header = ASR_FRAME_HEADER;
    frame->msgId = msgId;
    frame->msgLen = sizeof(cmd_uart_msg_t) + len + 1;
    frame->play = ret_status;
    frame->rsvd1 = 0;
    memcpy(frame->data, data, len);
    return frame;
}

int device_asr_free_frame(cmd_uart_msg_t* frame)
{
    if (frame != NULL)
        free(frame);

    return 0;
}

int device_asr_send_frame_default(cmd_uart_msg_t* frame)
{
    int ret;
    m_asr_uart.not_get_last_requst_reply = 1;
    ret = device_asr_send_frame_with_reply(frame, NULL, ASR_UART_DEFAULT_RETRY);
    return ret;
}

int device_asr_send_frame_default_with_reply(cmd_uart_msg_t* frame, cmd_uart_msg_t** reply)
{
    int ret;
    ret = device_asr_send_frame_with_reply(frame, reply, ASR_UART_DEFAULT_RETRY);
    return ret;
}

int device_asr_send_frame_default_no_wait(cmd_uart_msg_t* frame)
{
    int ret;
    ret = device_asr_send_frame_no_wait(frame);
    return ret;
}

int device_asr_send_frame(cmd_uart_msg_t* frame, uint8_t retryTimes)
{
    int ret;
    ret = device_asr_send_frame_with_reply(frame, NULL, retryTimes);
    return ret;
}

int device_asr_send_frame_no_wait(cmd_uart_msg_t* frame)
{
    int i = 0;
    int ret = 0;
    uint8_t sum = 0;
    struct timeval value;
    LOCK_UART()
    if (m_asr_uart.write_request != NULL) {
        UNLOCK_UART()
        return -1;
    }
    m_asr_uart.write_request = frame;
    m_asr_uart.requst_need_reply = 0;
    /*m_asr_uart.not_get_last_requst_reply = 0;*/
    /*calculate sum in case frame data is modified outside*/
    //for (i = 0; i < frame->msgLen - 1; ++i) {
    for (i = 0; i < frame->msgLen - 1; i++) {
        //printf("i = %d\n",i);
        sum += ((uint8_t*)frame)[i];
    }
    ((uint8_t*)frame)[frame->msgLen - 1] = sum;
    /*send request*/
//#if DEBUG_ASR_UART
#if 0
    printf("asr send frame:[%02X]\r\n", frame->msgId);
    for (i = 0; i < frame->msgLen; ++i) {
        printf("%02X ", ((uint8_t*)frame)[i]);
    }
    //printf("\n");
#endif
    /*check last active time*/
    uint64_t sleep_ticks = 0;
    if (m_asr_uart.last_active_ticks) {
        gettimeofday(&value,NULL);
        sleep_ticks = (uint64_t)(value.tv_sec * 1000 + value.tv_usec / 1000)
            - m_asr_uart.last_active_ticks;
        if (sleep_ticks < 50)
            usleep((50 - sleep_ticks)*1000);
    }
    ret = app_uart_write(device_asr_uart_fd, (uint8_t*)frame, frame->msgLen);
    if (ret<0) {
        UNLOCK_UART()
        return -1;
    }
    m_asr_uart.write_request = NULL;

    UNLOCK_UART()

    return 0;
}

int device_asr_send_frame_with_reply(cmd_uart_msg_t* frame,
        cmd_uart_msg_t** reply, uint8_t retryTimes)
{
    int ret;
    uint8_t sum = 0;
    int i = 0;
    struct timeval value;
    struct timespec outtime;

    LOCK_UART()
    if (m_asr_uart.write_request != NULL) {
        UNLOCK_UART()
        return -1;
    }
    m_asr_uart.write_request = frame;
    if (reply != NULL) {
        m_asr_uart.requst_need_reply = 1;
        m_asr_uart.not_get_last_requst_reply = 1;
    } else {
        m_asr_uart.requst_need_reply = 0;
        /*m_asr_uart.not_get_last_requst_reply = 0;*/
    }
    /*calculate sum in case frame data is modified outside*/
    for (i = 0; i < frame->msgLen - 1; ++i) {
        sum += ((uint8_t*)frame)[i];
    }
    ((uint8_t*)frame)[frame->msgLen - 1] = sum;

    /*send request*/
#if DEBUG_ASR_UART
    printf("asr send:[%02X]\n", frame->msgId);
    for (i = 0; i < frame->msgLen; ++i) {
        printf("%02X ", ((uint8_t*)frame)[i]);
    }
    printf("\n");
#endif
    /*check last active time*/
    uint64_t sleep_ticks = 0;
    LOGT("m_asr_uart.last_active_ticks = %llu", m_asr_uart.last_active_ticks);
    if (m_asr_uart.last_active_ticks) {
        gettimeofday(&value,NULL);
        sleep_ticks = (uint64_t)(value.tv_sec * 1000 + value.tv_usec / 1000)
            - m_asr_uart.last_active_ticks;
        LOGD("sleep_ticks = %llu", sleep_ticks);
        if (sleep_ticks < 50)
            usleep((50 - sleep_ticks) * 1000);
    }
    reply_timeout = TO1_MS;
    //memset(&ts, 0, sizeof(struct timespec));

    while (retryTimes > 0) {
        //struct timespec ts;
            long msecs = 0, add = 0;
        clock_gettime(CLOCK_REALTIME, &ts);
        //ts.tv_nsec += reply_timeout * 1000 * 1000;
        msecs = reply_timeout * 1000 * 1000 + ts.tv_nsec;
            add = msecs / (1000 * 1000 * 1000);
        //ts.tv_sec += 1;
        ts.tv_sec += add;
        ts.tv_nsec = msecs % (1000 * 1000 * 1000);
        LOGD("tv_sec = %ld, tv_nsec = %ld", ts.tv_sec, ts.tv_nsec);
        LOGD("reply_timeout = %d", reply_timeout);
        LOGD("retryTimes = %d", retryTimes);
        ret = app_uart_write(device_asr_uart_fd,
                (uint8_t*)frame, frame->msgLen);
        if (ret < 0) {
            UNLOCK_UART()
            LOGW("msg send fail...");
            return -1;
        }
        /* modify from sem_wait to sem_timedwait */
        /*ret = sem_wait(&m_asr_uart.write_sem);*/
        ret = sem_timedwait(&m_asr_uart.write_sem, &ts);
        /*gettimeofday(&value, NULL);
        outtime.tv_sec = value.tv_sec + 1;
        outtime.tv_nsec = value.tv_usec * 1000;
        ret = pthread_cond_timedwait(&m_asr_uart.write_signal, &m_asr_uart.lock,&outtime);*/
        if (ret == 0) {
            LOGD("recv UART reply OK");
            if (reply != NULL) {
                *reply = m_asr_uart.reply;
                m_asr_uart.reply = NULL;
                ret = 0;
            }
            break;
        } else {
            //sem_post(&m_asr_uart.write_sem);
            /*log_e("recv ASR reply timeout");*/
            //continue;
            retryTimes--;
            LOGD("retryTimes left = %d", retryTimes);
        }
    }

    m_asr_uart.write_request = NULL;
    m_asr_uart.requst_need_reply = 0;
    UNLOCK_UART()
    m_asr_uart.not_get_last_requst_reply = 0;
    /*log_raw("not_get_last_requst_reply==%d,%s,%d\r\n",
     * m_asr_uart.not_get_last_requst_reply,__FUNCTION__,__LINE__);
     * log_e("ret=====%d\n",ret);
     */
    return ret;
}


int device_asr_send_msg(ASR_UART_MSG_TYPE type, int arg)
{
    #ifdef test_asr_mode
    return ASR_AC_ERROR_NONE;
    #endif
    cmd_uart_msg_t* f = NULL;
    /*cmd_uart_msg_t* reply = NULL;*/
    int ret = ASR_AC_ERROR_NONE;
    float msg_type = arg;
    uint8_t byte_data[10] = {0x00};
    uint8_t flag = 0;

    if (type == ASR_UART_MSG_TYPE_CMD) {
        if (msg_type == ASR_UART_MSG_INC_T_BY_3) {
            LOGT("m_ac.setT:%f, m_ac_config.maxT:%f", m_ac.setT, m_ac_config.maxT);
            if ((m_ac.setT < m_ac_config.maxT) && (m_ac.setT >= (m_ac_config.maxT - 3))) {
                type =     ASR_UART_MSG_TYPE_SET_T;
                msg_type = 30;
            } else {
                type =     ASR_UART_MSG_TYPE_SET_T;
                msg_type = m_ac.setT + 3;
            }
            byte_data[0] = 0x03;
            flag = 0x01;
        } else if (msg_type == ASR_UART_MSG_DEC_T_BY_3) {
            LOGT("m_ac.setT:%f, m_ac_config.minT:%f", m_ac.setT, m_ac_config.minT);
            if ((m_ac.setT > m_ac_config.minT) && (m_ac.setT <= (m_ac_config.minT + 3))) {
                type =     ASR_UART_MSG_TYPE_SET_T;
                msg_type = 17;
            } else {
                type =     ASR_UART_MSG_TYPE_SET_T;
                msg_type = m_ac.setT - 3;
            }
            byte_data[0] = 0x04;
            flag = 0x01;
        } else if (msg_type == ASR_UART_MSG_INC_T_BY_1) {
            LOGT("m_ac.setT:%f, m_ac_config.maxT:%f", m_ac.setT, m_ac_config.maxT);
            if ((m_ac.setT < m_ac_config.maxT) && (m_ac.setT >= (m_ac_config.maxT - 1))) {
                type =     ASR_UART_MSG_TYPE_SET_T;
                msg_type = 30;
            } else {
                type =     ASR_UART_MSG_TYPE_SET_T;
                msg_type = m_ac.setT + 1;
            }
            byte_data[0] = 0x01;
            flag = 0x01;
        } else if (msg_type == ASR_UART_MSG_DEC_T_BY_1) {
            LOGT("m_ac.setT:%f, m_ac_config.minT:%f", m_ac.setT, m_ac_config.minT);
            if ((m_ac.setT > m_ac_config.minT) && (m_ac.setT <= (m_ac_config.minT + 1))) {
                type =     ASR_UART_MSG_TYPE_SET_T;
                msg_type = 17;
            } else {
                type =     ASR_UART_MSG_TYPE_SET_T;
                msg_type = m_ac.setT - 1;
            }
            byte_data[0] = 0x02;
            flag = 0x01;
        } else if (msg_type == ASR_UART_MSG_INC_HALFDEGREE_TEMP) {
            LOGT("m_ac.setT:%f, m_ac_config.maxT:%f", m_ac.setT, m_ac_config.maxT);
            if ((m_ac.setT < m_ac_config.maxT) && (m_ac.setT >= (m_ac_config.maxT - 0.5))) {
                type =     ASR_UART_MSG_TYPE_SET_T;
                msg_type = 30;
            } else {
                type =     ASR_UART_MSG_TYPE_SET_T;
                msg_type = m_ac.setT + 0.5;
            }
            byte_data[0] = 0x05;
            flag = 0x01;
        } else if (msg_type == ASR_UART_MSG_DEC_HALFDEGREE_TEMP) {
            LOGT("m_ac.setT:%f, m_ac_config.minT:%f", m_ac.setT, m_ac_config.minT);
            if ((m_ac.setT > m_ac_config.minT) && (m_ac.setT <= (m_ac_config.minT + 0.5))) {
                type =     ASR_UART_MSG_TYPE_SET_T;
                msg_type = 17;
            } else {
                type =     ASR_UART_MSG_TYPE_SET_T;
                msg_type = m_ac.setT - 0.5;
            }
            byte_data[0] = 0x06;
            flag = 0x01;
        } else if (msg_type == ASR_UART_MSG_SWING_UP_DOWN_ON) {
            byte_data[0] = 0x01;
            byte_data[1] = 0x00;
            m_ac.swing = ASR_AC_SWING_UP_DOWN_ON;
            LOGT("m_ac.swing = %d", m_ac.swing);
        } else if (msg_type == ASR_UART_MSG_SWING_UP_DOWN_OFF) {
            byte_data[0] = 0x02;
            byte_data[1] = 0x00;
            m_ac.swing = ASR_AC_SWING_UP_DOWN_OFF;
            LOGT("m_ac.swing = %d", m_ac.swing);
        } else if (msg_type == ASR_UART_MSG_SWING_LEFT_RIGHT_ON) {
            byte_data[0] = 0x00;
            byte_data[1] = 0x01;
            m_ac.swing = ASR_AC_SWING_LEFT_RIGHT_ON;
            LOGT("m_ac.swing = %d", m_ac.swing);
        } else if (msg_type == ASR_UART_MSG_SWING_LEFT_RIGHT_OFF) {
            byte_data[0] = 0x00;
            byte_data[1] = 0x02;
            m_ac.swing = ASR_AC_SWING_LEFT_RIGHT_OFF;
            LOGT("m_ac.swing = %d", m_ac.swing);
        } else if (msg_type == ASR_AC_WIND_DEC) {
            type =     ASR_UART_MSG_TYPE_SET_WIND;
        } else if (msg_type == ASR_AC_WIND_INC) {
            type =     ASR_UART_MSG_TYPE_SET_WIND;
        } else if (msg_type == ASR_UART_MSG_BLOW_UP) {
            byte_data[0] = 0x01;
            m_ac.blow = ASR_AC_BLOW_UP;
        } else if (msg_type == ASR_UART_MSG_BLOW_DOWN) {
            byte_data[0] = 0x02;
            m_ac.blow = ASR_AC_BLOW_DOWN;
        } else if (msg_type == ASR_UART_MSG_BLOW_LEFT) {
            byte_data[0] = 0x03;
            m_ac.blow = ASR_AC_BLOW_LEFT;
        } else if (msg_type == ASR_UART_MSG_BLOW_RIGHT) {
            byte_data[0] = 0x04;
            m_ac.blow = ASR_AC_BLOW_RIGHT;
        }
    }

    if (type == ASR_UART_MSG_TYPE_CMD) {
        uint8_t data[4];
        memset(data, 0, sizeof(data));

        if (msg_type == ASR_UART_MSG_WAKE_UP) {
            data[0] = 0x01;
        } else if (msg_type == ASR_UART_MSG_TIMEOUT_RECOG) {
            msg_type = ASR_UART_MSG_WAKE_UP;
            data[0] = 0x02;
        } else if (msg_type == ASR_UART_MSG_ON) {
            data[1] = 0x01;
        } else if (msg_type == ASR_UART_MSG_OFF) {
            msg_type = ASR_UART_MSG_ON;
            data[1] = 0x02;
        } else if ((msg_type == ASR_UART_MSG_SWING_UP_DOWN_ON) || (msg_type == ASR_UART_MSG_SWING_UP_DOWN_OFF)) {
            msg_type = ASR_UART_MSG_SET_SWING;
            data[1] = byte_data[0];
            data[2] = byte_data[1];
        } else if ((msg_type == ASR_UART_MSG_SWING_LEFT_RIGHT_ON) || (msg_type == ASR_UART_MSG_SWING_LEFT_RIGHT_OFF)) {
            msg_type = ASR_UART_MSG_SET_SWING;
            data[1] = byte_data[0];
            data[2] = byte_data[1];
        } else if ((msg_type == ASR_UART_MSG_BLOW_UP) || (msg_type == ASR_UART_MSG_BLOW_DOWN)) {
            msg_type = ASR_UART_MSG_SET_BLOW;
            data[0] = byte_data[0];
        } else if ((msg_type == ASR_UART_MSG_BLOW_LEFT) || (msg_type == ASR_UART_MSG_BLOW_RIGHT)) {
            msg_type = ASR_UART_MSG_SET_BLOW;
            data[0] = byte_data[0];
        }

        f = device_asr_alloc_frame(msg_type, data, sizeof(data));
        if (!f) {
            ret = ASR_AC_ERROR_MEM_FAIL;
            goto fail;
        }
        if (device_asr_send_frame_default(f) != 0) {
            ret = ASR_AC_ERROR_UART_TIMEOUT;
            goto fail;
        }
    } else if (type == ASR_UART_MSG_TYPE_QUERY_WEATHER) {
        uint8_t data[28];
        memset(data, 0, sizeof(data));
        data[5 - 5] = 0x01;

        f = device_asr_alloc_frame(msg_type, data, sizeof(data));
        if (!f) {
            ret = ASR_AC_ERROR_MEM_FAIL;
            goto fail;
        }
        if (device_asr_send_frame_default(f) != 0) {
            ret = ASR_AC_ERROR_UART_TIMEOUT;
            goto fail;
        }
    } else if (type == ASR_UART_MSG_TYPE_FACTORY) {
        uint8_t data[4];
        memset(data, 0, sizeof(data));
        if (msg_type == ASR_UART_MSG_FACTORY_TEST_PASS) {
            data[5 - 5] = 0x00;
        } else if (msg_type == ASR_UART_MSG_FACTORY_TEST_FAIL) {
            data[5 - 5] = 0x01;
        }
        f = device_asr_alloc_frame(ASR_UART_MSG_SET_FACTORY, data, sizeof(data));
        if (!f) {
            ret = ASR_AC_ERROR_MEM_FAIL;
            goto fail;
        }
        if (device_asr_send_frame_default(f) != 0) {
            ret = ASR_AC_ERROR_UART_TIMEOUT;
            goto fail;
        }
    } else if (type == ASR_UART_MSG_TYPE_SET_VOLUME) {
        uint8_t play_enable = 0x01;
        uint8_t data[4];
        //uint8_t data[10];
        uint8_t buffer[10];
        memset(data, 0, sizeof(data));
        //uint8_t ret_status = 0;
        //uint8_t data_len = sizeof(data);

        if (msg_type == ASR_UART_MSG_SET_VOLUME_MAX) {
            m_ac.volume = 100;
            data[5 - 5] = 0x01; //max volume
            data[6 - 5] = m_ac.volume;
            system("amixer cset name='head phone volume' 59");
            cmd_all_voice_reply_function("已设为", 6, play_enable);
            cmd_all_voice_reply_function("最大音量", 163, play_enable);
        } else if (msg_type == ASR_UART_MSG_SET_VOLUME_MIN) {
            m_ac.volume = 20;
            data[5 - 5] = 0x02; //min volume
            data[6 - 5] = m_ac.volume;
            system("amixer cset name='head phone volume' 46");
            cmd_all_voice_reply_function("已设为", 6, play_enable);
            cmd_all_voice_reply_function("最小音量", 164, play_enable);
        } else if ((msg_type == ASR_UART_MSG_SET_VOLUME_INC) || (msg_type == ASR_UART_MSG_SET_VOLUME_DEC)) {
            LOGT("m_ac.volume = %d\n", m_ac.volume);
	    uint8_t max_min_flag = 0;
            if (msg_type == ASR_UART_MSG_SET_VOLUME_INC) {
                if (m_ac.volume ==0) {
                    m_ac.volume = 20;
                } else if ((m_ac.volume >0) && (m_ac.volume <=20)) {
                    m_ac.volume = 40;
                } else if ((m_ac.volume >20) && (m_ac.volume <=40)) {
                    m_ac.volume = 60;
                } else if ((m_ac.volume >40) && (m_ac.volume <=60)) {
                    m_ac.volume = 80;
                } else if ((m_ac.volume >60) && (m_ac.volume <=80)) {
                    m_ac.volume = 100;
                } else if ((m_ac.volume >80) && (m_ac.volume <100)) {
                    m_ac.volume = 100;
                } else if (m_ac.volume >=100) {
                    m_ac.volume = 100;
		    max_min_flag = 1;
                }
            } else if (msg_type == ASR_UART_MSG_SET_VOLUME_DEC) {
                if (m_ac.volume <=20) {
                    m_ac.volume = 20;
		    max_min_flag = 2;
                } else if ((m_ac.volume >20) && (m_ac.volume <=40)) {
                    m_ac.volume = 20;
                } else if ((m_ac.volume >40) && (m_ac.volume <=60)) {
                    m_ac.volume = 40;
                } else if ((m_ac.volume >60) && (m_ac.volume <=80)) {
                    m_ac.volume = 60;
                } else if ((m_ac.volume >80) && (m_ac.volume <=100)) {
                    m_ac.volume = 80;
                }
            }

	    if (max_min_flag == 1) {
            	cmd_all_voice_reply_function("当前已经是", 14, play_enable);
            	cmd_all_voice_reply_function("最大音量", 163, play_enable);
	    } else if (max_min_flag == 2) {
            	cmd_all_voice_reply_function("当前已经是", 14, play_enable);
            	cmd_all_voice_reply_function("最小音量", 164, play_enable);
	    } else {
            	if ((m_ac.volume > 0) && (m_ac.volume <= 20)) {
                	system("amixer cset name='head phone volume' 46");
	                cmd_all_voice_reply_function("音量", 155, play_enable);
        	        cmd_all_voice_reply_function("20%", 200, play_enable);
                	//cmd_all_voice_reply_function("1", 119, play_enable);
	            } else if ((m_ac.volume > 20) && (m_ac.volume <= 40)) {
        	        system("amixer cset name='head phone volume' 49");
                	cmd_all_voice_reply_function("音量", 155, play_enable);
	                cmd_all_voice_reply_function("40%", 201, play_enable);
        	        //cmd_all_voice_reply_function("2", 120, play_enable);
	            } else if ((m_ac.volume > 40) && (m_ac.volume <= 60)) {
        	        system("amixer cset name='head phone volume' 53");
                	cmd_all_voice_reply_function("音量", 155, play_enable);
	                cmd_all_voice_reply_function("60%", 202, play_enable);
        	        //cmd_all_voice_reply_function("3", 121, play_enable);
	            } else if ((m_ac.volume > 60) && (m_ac.volume <= 80)) {
        	        system("amixer cset name='head phone volume' 55");
                	cmd_all_voice_reply_function("音量", 155, play_enable);
	               cmd_all_voice_reply_function("80%", 203, play_enable);
        	        //cmd_all_voice_reply_function("4", 122, play_enable);
	            } else if ((m_ac.volume > 80) && (m_ac.volume <= 100)) {
        	        system("amixer cset name='head phone volume' 59");
                	cmd_all_voice_reply_function("音量", 155, play_enable);
	                cmd_all_voice_reply_function("100%", 198, play_enable);
        	        //cmd_all_voice_reply_function("5", 123, play_enable);
	            }
	    }
            data[5 - 5] = 0x00; //no change
            data[6 - 5] = m_ac.volume; //set volume
        }
        sprintf(buffer, "%d", m_ac.volume);
        save_conf_info(FILE_OF_VOLUME_INFO, buffer);
        LOGD("m_ac.volume = %d", m_ac.volume);

        //data[5 - 5] = m_ac.volume;
        f = device_asr_alloc_frame(ASR_UART_MSG_SET_VOLUME, data, sizeof(data));
        if (!f) {
            ret = ASR_AC_ERROR_MEM_FAIL;
            goto fail;
        }
        if (device_asr_send_frame_default(f) != 0) {
            ret = ASR_AC_ERROR_UART_TIMEOUT;
            goto fail;
        }
    }  else if (type == ASR_UART_MSG_TYPE_SET_FUNC) {
        uint8_t data[26];
        memset(data, 0, sizeof(data));
        m_ac.self_clean = ASR_AC_SELF_CLEAN_DEFAULT;

        if (msg_type == ASR_UART_MSG_COMFORT_ECO_ON) {
            data[18 - 5] = 0x01;
            m_ac.confort_eco = ASR_AC_CONFORT_ECO_ON;
        } else if (msg_type == ASR_UART_MSG_COMFORT_ECO_OFF) {
            data[18 - 5] = 0x02;
            m_ac.confort_eco = ASR_AC_CONFORT_ECO_OFF;
        } else if (msg_type == ASR_UART_MSG_SELF_CLEAN_ON) {
            data[14 - 5] = 0x01;
            m_ac.self_clean = ASR_AC_SELF_CLEAN_ON;
        } else if (msg_type == ASR_UART_MSG_SELF_CLEAN_OFF) {
            data[14 - 5] = 0x02;
            m_ac.self_clean = ASR_AC_SELF_CLEAN_OFF;
        } else if (msg_type == ASR_UART_MSG_AUXILIARY_ELECTRIC_HEATING_ON) {
            data[12 - 5] = 0x01;
            m_ac.aux_electric_heat_mode = ASR_AC_AUX_ELECTRIC_HEAT_MODE_ON;
        } else if (msg_type == ASR_UART_MSG_AUXILIARY_ELECTRIC_HEATING_OFF) {
            data[12 - 5] = 0x02;
            m_ac.aux_electric_heat_mode = ASR_AC_AUX_ELECTRIC_HEAT_MODE_OFF;
        } else if (msg_type == ASR_UART_MSG_ANTI_BLOW_ON) {
            data[13 - 5] = 0x01;
            m_ac.anti_blow = ASR_AC_ANTI_BLOW_ON;
        } else if (msg_type == ASR_UART_MSG_ANTI_BLOW_OFF) {
            data[13 - 5] = 0x02;
            m_ac.anti_blow = ASR_AC_ANTI_BLOW_OFF;
        } else if (msg_type == ASR_UART_MSG_SCREEN_DISPLAY_ON) {
            data[9 - 5] = 0x01;
            m_ac.screen_display = ASR_AC_SCREEN_DISPLAY_ON;
        } else if (msg_type == ASR_UART_MSG_SCREEN_DISPLAY_OFF) {
            data[9 - 5] = 0x02;
            m_ac.screen_display = ASR_AC_SCREEN_DISPLAY_OFF;
        } else if (msg_type == ASR_UART_MSG_ANTI_COLD_ON) {
            data[19 - 5] = 0x01;
            m_ac.anti_cold = ASR_AC_ANTI_COLD_ON;
        } else if (msg_type == ASR_UART_MSG_ANTI_COLD_OFF) {
            data[19 - 5] = 0x02;
            m_ac.anti_cold = ASR_AC_ANTI_COLD_OFF;
        } else if (msg_type == ASR_UART_MSG_STRONG_MODE_ON) {
            data[11 - 5] = 0x01;
            m_ac.strong_mode = ASR_AC_STRONG_MODE_ON;
        } else if (msg_type == ASR_UART_MSG_STRONG_MODE_OFF) {
            data[11 - 5] = 0x02;
            m_ac.strong_mode = ASR_AC_STRONG_MODE_OFF;
        }

        f = device_asr_alloc_frame(ASR_UART_MSG_SET_FUNC, data, sizeof(data));
        if (!f) {
            ret = ASR_AC_ERROR_MEM_FAIL;
            goto fail;
        }
        if (device_asr_send_frame_default(f) != 0) {
            ret = ASR_AC_ERROR_UART_TIMEOUT;
            goto fail;
        }
        return ASR_AC_ERROR_NONE;
    } else if (type == ASR_UART_MSG_TYPE_SET_TIMER) {
	ASR_FRAME_MSG_ID timer_type = msg_type;
        uint16_t set_timer = 0xFFFF;
        uint8_t play_enable = 0x01;
        uint8_t data[10];
        memset(data, 0, sizeof(data));

        data[7 - 5] = 0xFF;
        data[6 - 5] = 0xFF;
	data[9 - 5]  = 0xFF;
	data[8 - 5]  = 0xFF;
        data[11 - 5] = 0xFF;
	data[10 - 5] = 0xFF;

	switch(timer_type) {
	case ASR_UART_MSG_TIMER_30M_ON:
		set_timer = 30;
		m_ac.timer_status = ASR_AC_TIMER_05_ON;
		break;
	case ASR_UART_MSG_TIMER_30M_OFF:
		set_timer = 30;
		m_ac.timer_status = ASR_AC_TIMER_05_OFF;
		break;

	case ASR_UART_MSG_TIMER_1H_ON:
		set_timer = 1 * 60;
		m_ac.timer_status = ASR_AC_TIMER_10_ON;
		break;
	case ASR_UART_MSG_TIMER_1H_OFF:
		set_timer = 1 * 60;
		m_ac.timer_status = ASR_AC_TIMER_10_OFF;
		break;

	case ASR_UART_MSG_TIMER_2H_ON:
		set_timer = 2 * 60;
		m_ac.timer_status = ASR_AC_TIMER_20_ON;
		break;
	case ASR_UART_MSG_TIMER_2H_OFF:
		set_timer = 2 * 60;
		m_ac.timer_status = ASR_AC_TIMER_20_OFF;
		break;

	case ASR_UART_MSG_TIMER_3H_ON:
		set_timer = 3 * 60;
		m_ac.timer_status = ASR_AC_TIMER_30_ON;
		break;
	case ASR_UART_MSG_TIMER_3H_OFF:
		set_timer = 3 * 60;
		m_ac.timer_status = ASR_AC_TIMER_30_OFF;
		break;

	case ASR_UART_MSG_TIMER_4H_ON:
		set_timer = 4 * 60;
		m_ac.timer_status = ASR_AC_TIMER_40_ON;
		break;
	case ASR_UART_MSG_TIMER_4H_OFF:
		set_timer = 4 * 60;
		m_ac.timer_status = ASR_AC_TIMER_40_OFF;
		break;

	case ASR_UART_MSG_TIMER_5H_ON:
		set_timer = 5 * 60;
		m_ac.timer_status = ASR_AC_TIMER_50_ON;
		break;
	case ASR_UART_MSG_TIMER_5H_OFF:
		set_timer = 5 * 60;
		m_ac.timer_status = ASR_AC_TIMER_50_OFF;
		break;

	case ASR_UART_MSG_TIMER_6H_ON:
		set_timer = 6 * 60;
		m_ac.timer_status = ASR_AC_TIMER_60_ON;
		break;
	case ASR_UART_MSG_TIMER_6H_OFF:
		set_timer = 6 * 60;
		m_ac.timer_status = ASR_AC_TIMER_60_OFF;
		break;

	case ASR_UART_MSG_TIMER_7H_ON:
		set_timer = 7 * 60;
		m_ac.timer_status = ASR_AC_TIMER_70_ON;
		break;
	case ASR_UART_MSG_TIMER_7H_OFF:
		set_timer = 7 * 60;
		m_ac.timer_status = ASR_AC_TIMER_70_OFF;
		break;

	case ASR_UART_MSG_TIMER_8H_ON:
		set_timer = 8 * 60;
		m_ac.timer_status = ASR_AC_TIMER_80_ON;
		break;
	case ASR_UART_MSG_TIMER_8H_OFF:
		set_timer = 8 * 60;
		m_ac.timer_status = ASR_AC_TIMER_80_OFF;
		break;

	case ASR_UART_MSG_TIMER_DISABLE:
		set_timer = 0x0;
		m_ac.timer_status = ASR_AC_TIMER_DISABLE;
		break;
	default:
		LOGW("invalid timer params!!!\n");
		break;
	}
/*
        if (device_ac_get()->onoff_status == 1) {
            data[7 - 5] = set_timer & 0x00FF;
            data[6 - 5] = (set_timer & 0xFF00) >> 8;
        } else if (device_ac_get()->onoff_status == 0) {
            data[7 - 5] = set_timer & 0x00FF;
            data[6 - 5] = (set_timer & 0xFF00) >> 8;
        }
        data[7 - 5] = set_timer & 0x00FF;
        data[6 - 5] = (set_timer & 0xFF00) >> 8;
*/
	if ((m_ac.timer_status == ASR_AC_TIMER_05_ON) || (m_ac.timer_status == ASR_AC_TIMER_10_ON) ||\
	    (m_ac.timer_status == ASR_AC_TIMER_20_ON) || (m_ac.timer_status == ASR_AC_TIMER_30_ON) ||\
	    (m_ac.timer_status == ASR_AC_TIMER_40_ON) || (m_ac.timer_status == ASR_AC_TIMER_50_ON) ||\
	    (m_ac.timer_status == ASR_AC_TIMER_60_ON) || (m_ac.timer_status == ASR_AC_TIMER_70_ON) ||\
	    (m_ac.timer_status == ASR_AC_TIMER_80_ON)) {
		data[8 - 5] = (set_timer & 0xFF00) >> 8;
                data[9 - 5] = set_timer & 0x00FF;
	} else if ((m_ac.timer_status == ASR_AC_TIMER_05_OFF) || (m_ac.timer_status == ASR_AC_TIMER_10_OFF) ||\
	    (m_ac.timer_status == ASR_AC_TIMER_20_OFF) || (m_ac.timer_status == ASR_AC_TIMER_30_OFF) ||\
	    (m_ac.timer_status == ASR_AC_TIMER_40_OFF) || (m_ac.timer_status == ASR_AC_TIMER_50_OFF) ||\
	    (m_ac.timer_status == ASR_AC_TIMER_60_OFF) || (m_ac.timer_status == ASR_AC_TIMER_70_OFF) ||\
	    (m_ac.timer_status == ASR_AC_TIMER_80_OFF)) {
		data[10 - 5] = (set_timer & 0xFF00) >> 8;
                data[11 - 5] = set_timer & 0x00FF;
	} else if (m_ac.timer_status == ASR_AC_TIMER_DISABLE) {
		data[6 - 5] = (set_timer & 0xFF00) >> 8;
                data[7 - 5] = set_timer & 0x00FF;
		data[8 - 5] = (set_timer & 0xFF00) >> 8;
                data[9 - 5] = set_timer & 0x00FF;
		data[10 - 5] = (set_timer & 0xFF00) >> 8;
                data[11 - 5] = set_timer & 0x00FF;
	}

        f = device_asr_alloc_frame(ASR_UART_MSG_SET_TIMER, data, sizeof(data));
        if (!f) {
            ret = ASR_AC_ERROR_MEM_FAIL;
            goto fail;
        }
        if (device_asr_send_frame_default(f) != 0) {
            ret = ASR_AC_ERROR_UART_TIMEOUT;
            goto fail;
        }
    } else if (type == ASR_UART_MSG_TYPE_SET_T) {
        uint8_t data[4];
        memset(data, 0, sizeof(data));
        int t = msg_type;
        if (msg_type >= m_ac_config.minT && msg_type <= m_ac_config.maxT) {
        //if (msg_type >= 17 && msg_type <= 30) {
            t = msg_type * 100;
        } else {
            t = 0x7FFF;
        }

        if (flag == 0x00) {
            data[0] = (uint8_t)(t & 0xFF);
            data[1] = (uint8_t)((t >> 8) & 0xFF);
        } else {
            data[0] = 0xFF;
            data[1] = 0x7F;
        }

        data[2] = byte_data[0];
        f = device_asr_alloc_frame(ASR_UART_MSG_SET_TEMP, data, sizeof(data));
        if (!f) {
            ret = ASR_AC_ERROR_MEM_FAIL;
            goto fail;
        }
        if (device_asr_send_frame_default(f) != 0) {
            ret = ASR_AC_ERROR_UART_TIMEOUT;
            goto fail;
        }
        //} else {
        //    ret = ASR_AC_ERROR_INVALID_PARAM;
        //    goto fail;
        //}
    } else if (type == ASR_UART_MSG_TYPE_SET_MODE) {
        uint8_t data[4];
        memset(data, 0, sizeof(data));
        int mode = msg_type;
        if (mode >= ASR_AC_MODE_MIN && mode <= ASR_AC_MODE_MAX) {
            data[0] = mode;
            f = device_asr_alloc_frame(ASR_UART_MSG_SET_MODE, data, sizeof(data));
            if (!f) {
                ret = ASR_AC_ERROR_MEM_FAIL;
                goto fail;
            }
            if (device_asr_send_frame_default(f) != 0) {
                ret = ASR_AC_ERROR_UART_TIMEOUT;
                goto fail;
            }
        } else {
            ret = ASR_AC_ERROR_INVALID_PARAM;
            goto fail;
        }
    } else if (type == ASR_UART_MSG_TYPE_PASSTHROUGH) {
        uint8_t data[10];
        uint8_t id[6];
        uint8_t dev_id_add_flag = 0x1;
        uint8_t data_len = sizeof(data);
        memset(data, 0, sizeof(data));
        memset(id, 0, sizeof(id));
        m_ac.asr_type = ASR_AC_ASR_TYPE_DEFAULT;

        if (msg_type == ASR_UART_MSG_LIVING_ROOM_FAN_ON) { //打开客厅风扇
            byte_data[0] = ASR_CMD_TYPE_SET_POWER_ON_OFF;
            byte_data[1] = ASR_LIVING_ROOM;
            byte_data[2] = ASR_OBJ_FAN;
            byte_data[9] = ASR_ACTION_POWERON;
            m_ac.asr_type = ASR_AC_ASR_TYPE_ON;
        } else if (msg_type == ASR_UART_MSG_LIVING_ROOM_FAN_OFF) { //关闭客厅风扇
            byte_data[0] = ASR_CMD_TYPE_SET_POWER_ON_OFF;
            byte_data[1] = ASR_LIVING_ROOM;
            byte_data[2] = ASR_OBJ_FAN;
            byte_data[9] = ASR_ACTION_POWEROFF;
            m_ac.asr_type = ASR_AC_ASR_TYPE_OFF;
        } else if (msg_type == ASR_UART_MSG_BED_ROOM_FAN_ON) { //打开卧室风扇
            byte_data[0] = ASR_CMD_TYPE_SET_POWER_ON_OFF;
            byte_data[1] = ASR_BED_ROOM;
            byte_data[2] = ASR_OBJ_FAN;
            byte_data[9] = ASR_ACTION_POWERON;
            m_ac.asr_type = ASR_AC_ASR_TYPE_ON;
        } else if (msg_type == ASR_UART_MSG_BED_ROOM_FAN_OFF) { //关闭卧室风扇
            byte_data[0] = ASR_CMD_TYPE_SET_POWER_ON_OFF;
            byte_data[1] = ASR_BED_ROOM;
            byte_data[2] = ASR_OBJ_FAN;
            byte_data[9] = ASR_ACTION_POWEROFF;
            m_ac.asr_type = ASR_AC_ASR_TYPE_OFF;
        } else if (msg_type == ASR_UART_MSG_LIVING_ROOM_FAN_OSC_ON) { //客厅风扇摇头
            byte_data[0] = ASR_CMD_TYPE_SET_FAN;
            byte_data[1] = ASR_LIVING_ROOM;
            byte_data[2] = ASR_OBJ_FAN;
            byte_data[9] = ASR_SET_FAN_OSC_ON;
            m_ac.asr_type = ASR_AC_ASR_TYPE_SET;
        } else if (msg_type == ASR_UART_MSG_BED_ROOM_FAN_OSC_ON) { //卧室风扇摇头
            byte_data[0] = ASR_CMD_TYPE_SET_FAN;
            byte_data[1] = ASR_BED_ROOM;
            byte_data[2] = ASR_OBJ_FAN;
            byte_data[9] = ASR_SET_FAN_OSC_ON;
            m_ac.asr_type = ASR_AC_ASR_TYPE_SET;
        } else if (msg_type == ASR_UART_MSG_LIVING_ROOM_FAN_OSC_OFF) { //客厅风扇停止摇头
            byte_data[0] = ASR_CMD_TYPE_SET_FAN;
            byte_data[1] = ASR_LIVING_ROOM;
            byte_data[2] = ASR_OBJ_FAN;
            byte_data[9] = ASR_SET_FAN_OSC_OFF;
            m_ac.asr_type = ASR_AC_ASR_TYPE_SET;
        } else if (msg_type == ASR_UART_MSG_BED_ROOM_FAN_OSC_OFF) { //卧室风扇停止摇头
            byte_data[0] = ASR_CMD_TYPE_SET_FAN;
            byte_data[1] = ASR_BED_ROOM;
            byte_data[2] = ASR_OBJ_FAN;
            byte_data[9] = ASR_SET_FAN_OSC_OFF;
            m_ac.asr_type = ASR_AC_ASR_TYPE_SET;
        } else if (msg_type == ASR_UART_MSG_LIVING_ROOM_FAN_INC) { //客厅风扇调大一档
            byte_data[0] = ASR_CMD_TYPE_SET_FAN;
            byte_data[1] = ASR_LIVING_ROOM;
            byte_data[2] = ASR_OBJ_FAN;
            byte_data[9] = ASR_SET_FAN_INC;
            m_ac.asr_type = ASR_AC_ASR_TYPE_SET;
        } else if (msg_type == ASR_UART_MSG_BED_ROOM_FAN_INC) { //卧室风扇调大一档
            byte_data[0] = ASR_CMD_TYPE_SET_FAN;
            byte_data[1] = ASR_BED_ROOM;
            byte_data[2] = ASR_OBJ_FAN;
            byte_data[9] = ASR_SET_FAN_INC;
            m_ac.asr_type = ASR_AC_ASR_TYPE_SET;
        } else if (msg_type == ASR_UART_MSG_LIVING_ROOM_FAN_DEC) { //客厅风扇调小一档
            byte_data[0] = ASR_CMD_TYPE_SET_FAN;
            byte_data[1] = ASR_LIVING_ROOM;
            byte_data[2] = ASR_OBJ_FAN;
            byte_data[9] = ASR_SET_FAN_DEC;
            m_ac.asr_type = ASR_AC_ASR_TYPE_SET;
        } else if (msg_type == ASR_UART_MSG_BED_ROOM_FAN_DEC) { //卧室风扇调小一档
            byte_data[0] = ASR_CMD_TYPE_SET_FAN;
            byte_data[1] = ASR_BED_ROOM;
            byte_data[2] = ASR_OBJ_FAN;
            byte_data[9] = ASR_SET_FAN_DEC;
            m_ac.asr_type = ASR_AC_ASR_TYPE_SET;
        } else if (msg_type == ASR_UART_MSG_LIVING_ROOM_FAN_MAX) { //客厅风扇调到最大
            byte_data[0] = ASR_CMD_TYPE_SET_FAN;
            byte_data[1] = ASR_LIVING_ROOM;
            byte_data[2] = ASR_OBJ_FAN;
            byte_data[9] = ASR_SET_FAN_MAX;
            m_ac.asr_type = ASR_AC_ASR_TYPE_SET;
        } else if (msg_type == ASR_UART_MSG_BED_ROOM_FAN_MAX) { //卧室风扇调到最大
            byte_data[0] = ASR_CMD_TYPE_SET_FAN;
            byte_data[1] = ASR_BED_ROOM;
            byte_data[2] = ASR_OBJ_FAN;
            byte_data[9] = ASR_SET_FAN_MAX;
            m_ac.asr_type = ASR_AC_ASR_TYPE_SET;
        } else if (msg_type == ASR_UART_MSG_LIVING_ROOM_FAN_MIN) { //客厅风扇调到最小
            byte_data[0] = ASR_CMD_TYPE_SET_FAN;
            byte_data[1] = ASR_LIVING_ROOM;
            byte_data[2] = ASR_OBJ_FAN;
            byte_data[9] = ASR_SET_FAN_MIN;
            m_ac.asr_type = ASR_AC_ASR_TYPE_SET;
        } else if (msg_type == ASR_UART_MSG_BED_ROOM_FAN_MIN) { //卧室风扇调到最小
            byte_data[0] = ASR_CMD_TYPE_SET_FAN;
            byte_data[1] = ASR_BED_ROOM;
            byte_data[2] = ASR_OBJ_FAN;
            byte_data[9] = ASR_SET_FAN_MIN;
            m_ac.asr_type = ASR_AC_ASR_TYPE_SET;
        } else if (msg_type == ASR_UART_MSG_LIVING_ROOM_HUMIDIFIER_ON) { //打开客厅加湿器
            byte_data[0] = ASR_CMD_TYPE_SET_POWER_ON_OFF;
            byte_data[1] = ASR_LIVING_ROOM;
            byte_data[2] = ASR_OBJ_HUMIDIFIER;
            byte_data[9] = ASR_ACTION_POWERON;
            m_ac.asr_type = ASR_AC_ASR_TYPE_ON;
        } else if (msg_type == ASR_UART_MSG_BED_ROOM_HUMIDIFIER_ON) { //打开卧室加湿器
            byte_data[0] = ASR_CMD_TYPE_SET_POWER_ON_OFF;
            byte_data[1] = ASR_BED_ROOM;
            byte_data[2] = ASR_OBJ_HUMIDIFIER;
            byte_data[9] = ASR_ACTION_POWERON;
            m_ac.asr_type = ASR_AC_ASR_TYPE_ON;
        } else if (msg_type == ASR_UART_MSG_LIVING_ROOM_HUMIDIFIER_OFF) { //关闭客厅加湿器
            byte_data[0] = ASR_CMD_TYPE_SET_POWER_ON_OFF;
            byte_data[1] = ASR_LIVING_ROOM;
            byte_data[2] = ASR_OBJ_HUMIDIFIER;
            byte_data[9] = ASR_ACTION_POWEROFF;
            m_ac.asr_type = ASR_AC_ASR_TYPE_OFF;
        } else if (msg_type == ASR_UART_MSG_BED_ROOM_HUMIDIFIER_OFF) { //关闭卧室加湿器
            byte_data[0] = ASR_CMD_TYPE_SET_POWER_ON_OFF;
            byte_data[1] = ASR_BED_ROOM;
            byte_data[2] = ASR_OBJ_HUMIDIFIER;
            byte_data[9] = ASR_ACTION_POWEROFF;
            m_ac.asr_type = ASR_AC_ASR_TYPE_OFF;
        } else if (msg_type == ASR_UART_MSG_LIVING_ROOM_AIR_CLEANER_ON) { //打开客厅净化器
            byte_data[0] = ASR_CMD_TYPE_SET_POWER_ON_OFF;
            byte_data[1] = ASR_LIVING_ROOM;
            byte_data[2] = ASR_OBJ_AIR_CLEANER;
            byte_data[9] = ASR_ACTION_POWERON;
            m_ac.asr_type = ASR_AC_ASR_TYPE_ON;
        } else if (msg_type == ASR_UART_MSG_BED_ROOM_AIR_CLEANER_ON) { //打开卧室净化器
            byte_data[0] = ASR_CMD_TYPE_SET_POWER_ON_OFF;
            byte_data[1] = ASR_BED_ROOM;
            byte_data[2] = ASR_OBJ_AIR_CLEANER;
            byte_data[9] = ASR_ACTION_POWERON;
            m_ac.asr_type = ASR_AC_ASR_TYPE_ON;
        } else if (msg_type == ASR_UART_MSG_LIVING_ROOM_AIR_CLEANER_OFF) { //关闭客厅净化器
            byte_data[0] = ASR_CMD_TYPE_SET_POWER_ON_OFF;
            byte_data[1] = ASR_LIVING_ROOM;
            byte_data[2] = ASR_OBJ_AIR_CLEANER;
            byte_data[9] = ASR_ACTION_POWEROFF;
            m_ac.asr_type = ASR_AC_ASR_TYPE_OFF;
        } else if (msg_type == ASR_UART_MSG_BED_ROOM_AIR_CLEANER_OFF) { //关闭卧室净化器
            byte_data[0] = ASR_CMD_TYPE_SET_POWER_ON_OFF;
            byte_data[1] = ASR_BED_ROOM;
            byte_data[2] = ASR_OBJ_AIR_CLEANER;
            byte_data[9] = ASR_ACTION_POWEROFF;
            m_ac.asr_type = ASR_AC_ASR_TYPE_OFF;
        } else if (msg_type == ASR_UART_MSG_SWEEP_ROBOT_CLEAN_ALL) { //扫地机器人全屋清扫
            byte_data[0] = ASR_CMD_TYPE_SET_SWEEP_ROBOT;
            byte_data[1] = ASR_SET_SWEEP_ROBOT_CLEAN_ALL;
            dev_id_add_flag = 0x0;
            data_len = 0x02;
            m_ac.asr_type = ASR_AC_ASR_TYPE_ROBOT_CLEAN;//ASR_AC_ASR_TYPE_SET;
        } else if (msg_type == ASR_UART_MSG_SWEEP_ROBOT_CHARGING) { //扫地机器人充电
            byte_data[0] = ASR_CMD_TYPE_SET_SWEEP_ROBOT;
            byte_data[1] = ASR_SET_SWEEP_ROBOT_CHARGING;
            dev_id_add_flag = 0x0;
            data_len = 0x02;
            m_ac.asr_type = ASR_AC_ASR_TYPE_ROBOT_CHARGING;//ASR_AC_ASR_TYPE_SET;
        } else if (msg_type == ASR_UART_MSG_SWEEP_ROBOT_CLEAN_PAUSE) { //扫地机器人暂停扫地
            byte_data[0] = ASR_CMD_TYPE_SET_SWEEP_ROBOT;
            byte_data[1] = ASR_SET_SWEEP_ROBOT_PAUSE;
            dev_id_add_flag = 0x0;
            data_len = 0x02;
            m_ac.asr_type = ASR_AC_ASR_TYPE_ROBOT_PAUSE;//ASR_AC_ASR_TYPE_SET;
        } else if (msg_type == ASR_UART_MSG_BED_ROOM_AC_ON) { //打开卧室空调
            byte_data[0] = ASR_CMD_TYPE_SET_POWER_ON_OFF;
            byte_data[1] = ASR_BED_ROOM;
            byte_data[2] = ASR_OBJ_AC;
            byte_data[9] = ASR_ACTION_POWERON;
            m_ac.asr_type = ASR_AC_ASR_TYPE_ON;
        } else if (msg_type == ASR_UART_MSG_BED_ROOM_AC_OFF) { //关闭卧室空调
            byte_data[0] = ASR_CMD_TYPE_SET_POWER_ON_OFF;
            byte_data[1] = ASR_BED_ROOM;
            byte_data[2] = ASR_OBJ_AC;
            byte_data[9] = ASR_ACTION_POWEROFF;
            m_ac.asr_type = ASR_AC_ASR_TYPE_OFF;
        } else if (msg_type == ASR_UART_MSG_LIVING_ROOM_LAMP_ON) { //客厅开灯
            byte_data[0] = ASR_CMD_TYPE_SET_POWER_ON_OFF;
            byte_data[1] = ASR_LIVING_ROOM;
            byte_data[2] = ASR_OBJ_SMART_LAMP;
            byte_data[9] = ASR_ACTION_POWERON;
            m_ac.asr_type = ASR_AC_ASR_TYPE_ON;
        } else if (msg_type == ASR_UART_MSG_LIVING_ROOM_LAMP_OFF) { //客厅关灯
            byte_data[0] = ASR_CMD_TYPE_SET_POWER_ON_OFF;
            byte_data[1] = ASR_LIVING_ROOM;
            byte_data[2] = ASR_OBJ_SMART_LAMP;
            byte_data[9] = ASR_ACTION_POWEROFF;
            m_ac.asr_type = ASR_AC_ASR_TYPE_OFF;
        } else if (msg_type == ASR_UART_MSG_BED_ROOM_LAMP_ON) { //卧室开灯
            byte_data[0] = ASR_CMD_TYPE_SET_POWER_ON_OFF;
            byte_data[1] = ASR_BED_ROOM;
            byte_data[2] = ASR_OBJ_SMART_LAMP;
            byte_data[9] = ASR_ACTION_POWERON;
            m_ac.asr_type = ASR_AC_ASR_TYPE_ON;
        } else if (msg_type == ASR_UART_MSG_BED_ROOM_LAMP_OFF) { //卧室关灯
            byte_data[0] = ASR_CMD_TYPE_SET_POWER_ON_OFF;
            byte_data[1] = ASR_BED_ROOM;
            byte_data[2] = ASR_OBJ_SMART_LAMP;
            byte_data[9] = ASR_ACTION_POWEROFF;
            m_ac.asr_type = ASR_AC_ASR_TYPE_OFF;
        } else if (msg_type == ASR_UART_MSG_QUERY_WEATHER) { //天气查询
            byte_data[0] = ASR_CMD_TYPE_SET_QUERY_WEATHER;
            dev_id_add_flag = 0x0;
            data_len = 0x01;
            m_ac.asr_type = ASR_AC_ASR_TYPE_WEATHER;
        } else if (msg_type == ASR_UART_MSG_BACK_HOME) { //回家
            byte_data[0] = ASR_CMD_TYPE_SET_I;
            byte_data[1] = ASR_SET_I_TYPE_BACK_HOME;
            dev_id_add_flag = 0x0;
            data_len = 0x02;
            m_ac.asr_type = ASR_AC_ASR_TYPE_BACK_HOME;
        } else if (msg_type == ASR_UART_MSG_LEAVE_HOME) { //离家
            byte_data[0] = ASR_CMD_TYPE_SET_I;
            byte_data[1] = ASR_SET_I_TYPE_LEAVE_HOME;
            dev_id_add_flag = 0x0;
            data_len = 0x02;
            m_ac.asr_type = ASR_AC_ASR_TYPE_LEAVE_HOME;
        }

        if (dev_id_add_flag == 0x1) {
            if (get_device_match_id(byte_data[1], byte_data[2], id) == 0) {
                byte_data[3] = id[0];
                byte_data[4] = id[1];
                byte_data[5] = id[2];
                byte_data[6] = id[3];
                byte_data[7] = id[4];
                byte_data[8] = id[5];
            } else {
                ret = ASR_AC_ERROR_NO_DEVCONFIG;
                goto fail;
            }
        }

        if ((m_ac.asr_type == ASR_AC_ASR_TYPE_OFF) || (m_ac.asr_type == ASR_AC_ASR_TYPE_ON) || \
            (m_ac.asr_type == ASR_AC_ASR_TYPE_SET) || (m_ac.asr_type == ASR_AC_ASR_TYPE_ROBOT_CLEAN) || \
	    (m_ac.asr_type == ASR_AC_ASR_TYPE_ROBOT_PAUSE) || (m_ac.asr_type == ASR_AC_ASR_TYPE_ROBOT_CHARGING)) {
            gettimeofday(&timer_start,NULL);
            eii_ctl_timeout_flag = 0x01;
        }

        for (int i = 0; i < sizeof(data); i++) {
            data[i] = byte_data[i];
        }

        f = device_asr_alloc_frame(ASR_UART_MSG_PASSTHROUGH, data, data_len);
        if (!f) {
            ret = ASR_AC_ERROR_MEM_FAIL;
            goto fail;
        }

        if (device_asr_send_frame_default(f) != 0) {
            ret = ASR_AC_ERROR_UART_TIMEOUT;
            goto fail;
        }
    } else if (type == ASR_UART_MSG_TYPE_SET_WIND) {
        uint8_t data[4];
        memset(data, 0, sizeof(data));
        int wind = msg_type;
        m_ac.wind_play_type = 0;

        if (wind == ASR_AC_WIND_SLOW) {
            m_ac.wind_speed = 1;
	    m_ac.wind_play_type = 1;
        } else if (wind == ASR_AC_WIND_MID) {
            m_ac.wind_speed = 60;
	    m_ac.wind_play_type = 1;
        } else if (wind == ASR_AC_WIND_HIGH) {
            m_ac.wind_speed = 100;
	    m_ac.wind_play_type = 1;
        } else if (wind == ASR_AC_WIND_AUTO) {
            m_ac.wind_speed = 102;
        } else if (wind == ASR_AC_WIND_INC) {
/*
            if (m_ac.wind_speed == 1) {
                m_ac.wind_speed = 20;
            } else if ((m_ac.wind_speed > 1) && (m_ac.wind_speed <= 20)) {
                m_ac.wind_speed = 40;
            } else if ((m_ac.wind_speed > 20) && (m_ac.wind_speed <= 40)) {
                m_ac.wind_speed = 60;
            } else if ((m_ac.wind_speed > 40) && (m_ac.wind_speed <= 60)) {
                m_ac.wind_speed = 80;
            } else if ((m_ac.wind_speed > 60) && (m_ac.wind_speed < 100)) {
                m_ac.wind_speed = 100;
            } else if (m_ac.wind_speed == 102) {
                m_ac.wind_speed = 100;
            }
*/
            byte_data[1] = 0x01;
        } else if (wind == ASR_AC_WIND_DEC) {
/*
            if (m_ac.wind_speed == 100) {
                m_ac.wind_speed = 80;
            } else if ((m_ac.wind_speed >= 80) && (m_ac.wind_speed < 100)) {
                m_ac.wind_speed = 60;
            } else if ((m_ac.wind_speed >=60 ) && (m_ac.wind_speed < 80)) {
                m_ac.wind_speed = 40;
            } else if ((m_ac.wind_speed >= 40) && (m_ac.wind_speed < 60)) {
                m_ac.wind_speed = 20;
            } else if ((m_ac.wind_speed > 1) && (m_ac.wind_speed < 40)) {
                m_ac.wind_speed = 1;
            } else if (m_ac.wind_speed == 102) {
                m_ac.wind_speed = 1;
            }
*/
            byte_data[1] = 0x02;
        }

        if ((byte_data[1] == 0x01) || (byte_data[1] == 0x02)) {
            byte_data[0] = 0x0;
        } else {
            byte_data[0] = m_ac.wind_speed;
        }
        LOGT("byte_data[0] = %d, m_ac.wind_play_type = %d", byte_data[0], m_ac.wind_play_type);

        data[0] = byte_data[0];
        data[2] = byte_data[1];
        f = device_asr_alloc_frame(ASR_UART_MSG_WIND_SPEED, data, sizeof(data));
        if (!f) {
            ret = ASR_AC_ERROR_MEM_FAIL;
            goto fail;
        }
        if (device_asr_send_frame_default(f) != 0) {
            ret = ASR_AC_ERROR_UART_TIMEOUT;
            goto fail;
        }
    } else if (type == ASR_UART_MSG_TYPE_ACK_BOARD_STD) {
        uint8_t data[26];
        uint8_t ret_status = 0;
        uint8_t data_len = sizeof(data);
        memset(data, 0, sizeof(data));

        if (msg_type == ASR_UART_MSG_BOARD_STARTUP_PLAY) {
            ret_status = 0x01;
            data[0] = 0x0;
            data_len = 4;
        } else if (msg_type == ASR_UART_MSG_BOARD_FACTORY_TEST) {
            ret_status = 0x01;
            cmd_all_voice_reply_function("切换生产模式", 800, 0x01);
            data_len = 4;
            data[5 - 5] = 0x01;
/*
        ret = pthread_create(&factoryThread, NULL, factory_test_thread, NULL);
        if (ret != 0)
        {
            LOGE("factoryThread create error [ret = %d]", ret);
            return -1;
        }

        system("amixer cset name='head phone volume' 54");
*/
        } else if (msg_type == ASR_UART_MSG_BOARD_WIFI_STATUS) {
            ret_status = 0x01;
            data_len = 20;
        } else if (msg_type == ASR_UART_MSG_BOARD_SET_CONFIGS) {
            ret_status = 0x00;
            data[5 - 5] = 0xFF;
            data[8 - 5] = m_ac.volume;
            data[9 - 5] = (device_ac_get()->local_dev_set_asr | (device_ac_get()->local_dev_set_play) << 1);
            data_len = 10;
            LOGT("m_ac.local_dev_set_asr = %#x", m_ac.local_dev_set_asr);
            LOGT("m_ac.local_dev_set_play = %#x", m_ac.local_dev_set_play);
            LOGT("device_ac_get()->local_dev_set_asr = %#x", device_ac_get()->local_dev_set_asr);
            LOGT("device_ac_get()->local_dev_set_play = %#x", device_ac_get()->local_dev_set_play);
            LOGT("data[9 - 5] = %#x", data[9 - 5]);
        } else if (msg_type == ASR_UART_MSG_BOARD_WIFI_PASSTHROUGH) {
            ret_status = 0x01;
            data_len = 4;
        } else if (msg_type == ASR_UART_MSG_BOARD_BEEP_NOTICE) {
            ret_status = 0x00;
            data_len = 4;
        } else if (msg_type == ASR_UART_MSG_PASSTHROUGH) {
            data[0] = ASR_CMD_TYPE_SET_EII_DEV_STATUS;
            data[1] = (m_ac.eii_dev_set_asr << 1) | (m_ac.eii_dev_set_play);
            data_len = 2;
        } else {
            LOGW("should't see me!!!");
            return -1;
        }

        f = device_asr_alloc_frame_ack_board(msg_type, data, data_len, ret_status);
        if (!f) {
            ret = ASR_AC_ERROR_MEM_FAIL;
            goto fail;
        }
        //if (device_asr_send_frame_default(f) != 0) {
        if (device_asr_send_frame_default_no_wait(f) != 0) {
            ret = ASR_AC_ERROR_UART_TIMEOUT;
            goto fail;
        }
    } else {
        LOGW("should't see me!!!");
    }
fail:
    if (type != ASR_UART_MSG_TYPE_SET_VOLUME) {
        if (f)
        {
            device_asr_free_frame(f);
        }
    }
    if (ret == ASR_AC_ERROR_NONE) {
        if (!m_ac.play) {
            ret = ASR_AC_ERROR_NO_PLAY;
        } else {
            ret = m_ac.err;
            m_ac.err = ASR_AC_ERROR_NONE;
        }
    }
    return ret;
}
