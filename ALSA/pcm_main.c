/*
	播放随机数据，播放出来是一段噪声。
*/
#include "inlcude/asoundlib.h"

static char *device="default";  //播放音频的逻辑设备
snd_output_t *output = NULL;        //未用
unsigned char buffer[16*1024];    //音频数据存储缓冲区

int main(void)
{
	int err;
	unsigned int i;
	snd_pcm_t *handle;          //播放句柄，类似文件描述符fd.标识一个播放
	snd_pcm_sframes_t frames;      //已经播放的帧数
	for (i = 0;i < sizeof(buffer); i++)//生成播放随机数据
		buffer[i] = random() & 0xff;
	//打开播放流设备
	if((err = snd_pcm_open(&handle,device,SND_PCM_STREAM_PLAYBACK,0)) < 0) {
		printf("playback open error:%s\n",snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	/*
		设置播放参数
		int snd_pcm_set_params	(	snd_pcm_t * 	pcm,
					snd_pcm_format_t 	format,
					snd_pcm_access_t 	access, //数据存储方式，一般是交错
					unsigned int 	channels,//播放通道数
					unsigned int 	rate,//播放率
					int 	soft_resample,
					//重采样（Resampling）指的是把时间序列的频度变为另一个频度的过程。
					把高频度的数据变为低频度叫做降采样（downsampling），把低频度变为高频度叫做增采样
					（upsampling）。
					unsigned int 	latency //播放延时us
					)	
					
	*/
	if((err = snd_pcm_set_params(handle,SND_PCM_FORMAT_U8,SND_PCM_ACCESS_RW_INTERLEAVED,1,48000,1,500000))<0){
		printf("Playback open error:%s\n",snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	for (i = 0; i < 16; i++) {
                frames = snd_pcm_writei(handle, buffer, sizeof(buffer));//播放
                if (frames < 0)
                        frames = snd_pcm_recover(handle, frames, 0);//从错误帧处恢复播放
                if (frames < 0) {
                        printf("snd_pcm_writei failed: %s\n", snd_strerror(frames));
                        break;
                }
                if (frames > 0 && frames < (long)sizeof(buffer))
                        printf("Short write (expected %li, wrote %li)\n", (long)sizeof(buffer), frames);
        }
    snd_pcm_close(handle);//关闭播放流
    return 0;
}

