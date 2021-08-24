#include <string.h>
#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/mman.h>
#include <android/log.h>

#include <errno.h>


jint
Java_ac_kr_kgu_esproject_ArrayAdderActivity_SegmentControl (JNIEnv* env, jobject thiz, jint data )
{	
	int dev, ret ;
	dev = open("/dev/segment",O_RDWR | O_SYNC);

	if(dev != -1) {
		ret = write(dev,&data,4);
		close(dev);
	} else {
		// 
		__android_log_print(ANDROID_LOG_ERROR, "SegmentActivity", "Device Open ERROR!\n");
		exit(1);
	}
	return 0;
}

jint
Java_ac_kr_kgu_esproject_ArrayAdderActivity_ArraySum (JNIEnv* env, jobject thiz, jint sumValue, jintArray sumArray )
{	
	int dev, ret ;
	int sumResult = 0;
	int i;
	int arrayLen = (*env)->GetArrayLength(env, sumArray);
	int * arrayCopy = (*env)->GetIntArrayElements(env, sumArray, 0);

	//
	for(i = 0 ; i < arrayLen ; i++){
		sumResult += arrayCopy[i];
	}
	//
	if(sumResult != sumValue){
		return -1;
	}
	else{
		return 0;	
	}
}


jint
Java_ac_kr_kgu_esproject_ArrayAdderActivity_SegmentIOControl (JNIEnv* env, jobject thiz, jint data )
{
	int dev, ret ;
	dev = open("/dev/segment",O_RDWR | O_SYNC);

	if(dev != -1) {
		ret = ioctl(dev, data, NULL, NULL);
		close(dev);
	} else {
		__android_log_print(ANDROID_LOG_ERROR, "SegmentActivity", "Device Open ERROR!\n");
		exit(1);
	}
	return 0;
}

jint
Java_ac_kr_kgu_esproject_ArrayAdderActivity_BuzzerControl( JNIEnv* env, jobject thiz, jint value )
{
	int fd,ret;
	int data = value;

	fd = open("/dev/buzzer",O_WRONLY);
	
	if(fd < 0) return -errno;
	
	ret = write(fd, &data, 1);
	close(fd);
	
	if(ret == 1) return 0;
	
	return -1;
}

