/*
	����������ݣ����ų�����һ��������
*/
#include "inlcude/asoundlib.h"

static char *device="default";  //������Ƶ���߼��豸
snd_output_t *output = NULL;        //δ��
unsigned char buffer[16*1024];    //��Ƶ���ݴ洢������

int main(void)
{
	int err;
	unsigned int i;
	snd_pcm_t *handle;          //���ž���������ļ�������fd.��ʶһ������
	snd_pcm_sframes_t frames;      //�Ѿ����ŵ�֡��
	for (i = 0;i < sizeof(buffer); i++)//���ɲ����������
		buffer[i] = random() & 0xff;
	//�򿪲������豸
	if((err = snd_pcm_open(&handle,device,SND_PCM_STREAM_PLAYBACK,0)) < 0) {
		printf("playback open error:%s\n",snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	/*
		���ò��Ų���
		int snd_pcm_set_params	(	snd_pcm_t * 	pcm,
					snd_pcm_format_t 	format,
					snd_pcm_access_t 	access, //���ݴ洢��ʽ��һ���ǽ���
					unsigned int 	channels,//����ͨ����
					unsigned int 	rate,//������
					int 	soft_resample,
					//�ز�����Resampling��ָ���ǰ�ʱ�����е�Ƶ�ȱ�Ϊ��һ��Ƶ�ȵĹ��̡�
					�Ѹ�Ƶ�ȵ����ݱ�Ϊ��Ƶ�Ƚ�����������downsampling�����ѵ�Ƶ�ȱ�Ϊ��Ƶ�Ƚ���������
					��upsampling����
					unsigned int 	latency //������ʱus
					)	
					
	*/
	if((err = snd_pcm_set_params(handle,SND_PCM_FORMAT_U8,SND_PCM_ACCESS_RW_INTERLEAVED,1,48000,1,500000))<0){
		printf("Playback open error:%s\n",snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	for (i = 0; i < 16; i++) {
                frames = snd_pcm_writei(handle, buffer, sizeof(buffer));//����
                if (frames < 0)
                        frames = snd_pcm_recover(handle, frames, 0);//�Ӵ���֡���ָ�����
                if (frames < 0) {
                        printf("snd_pcm_writei failed: %s\n", snd_strerror(frames));
                        break;
                }
                if (frames > 0 && frames < (long)sizeof(buffer))
                        printf("Short write (expected %li, wrote %li)\n", (long)sizeof(buffer), frames);
        }
    snd_pcm_close(handle);//�رղ�����
    return 0;
}

