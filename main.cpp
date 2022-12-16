#pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )
//#pragma comment( linker, "/subsystem:windows /entry:WinMainCRTStartup" )
//#pragma comment( linker, "/subsystem:console /entry:mainCRTStartup" )
//#pragma comment( linker, "/subsystem:console /entry:WinMainCRTStartup" )

#include "windows.h"
#include "iostream"
#include <tchar.h>
#include "jni.h"


using namespace std;

typedef jint(JNICALL* JNICREATEPROC)(JavaVM**, void**, void*);

bool startJVM();
//
//int APIENTRY WinMain(HINSTANCE hInstance,
//                     HINSTANCE hPrevInstance,
//                     LPSTR      lpCmdLine,
//                     int        nCmdShow){
//    startJVM();
//    return 0;
//}

int main(int argc, _TCHAR* argv[]){
    startJVM();
    return 0;
}

//启动java虚拟机
bool startJVM() {

    //获取应用程序目录
    char szapipath[MAX_PATH];//（D:\Documents\Downloads\TEST.exe）
    memset(szapipath,0,MAX_PATH);
    GetModuleFileNameA(NULL,szapipath,MAX_PATH);

    //获取应用程序名称
    char szExe[MAX_PATH] = "";//（TEST.exe）
    char *pbuf = NULL;
    char* szLine = strtok_s(szapipath,"\\",&pbuf);
    while (NULL != szLine)
    {
        strcpy_s(szExe, szLine);
        szLine = strtok_s(NULL,"\\",&pbuf);
    }
    //删除.exe
//    strncpy_s(szapipath, szExe, strlen(szExe)-4);
    //获取jvm动态库的路径
    TCHAR* jvmPath = _T(".\\jre\\bin\\server\\jvm.dll");

    //java虚拟机启动时接收的参数，每个参数单独一项
    int nOptionCount = 2;
    JavaVMOption vmOption[nOptionCount];
    //设置JVM最大允许分配的堆内存，按需分配
    string path = "-Djava.class.path=";
    path.append(szExe);
    //D:\workspace\MqttTest\TestDemo\target\test
//    vmOption[0].optionString = (char*)path.data();
    vmOption[0].optionString = "-Djava.class.path=D:\\workspace\\MqttTest\\TestDemo\\target\\test\\test.jar";
    //设置classpath
    vmOption[1].optionString = "-Xmx1024M";

    JavaVMInitArgs vmInitArgs;
    vmInitArgs.version = JNI_VERSION_10;
    vmInitArgs.options = vmOption;
    vmInitArgs.nOptions = nOptionCount;
    //忽略无法识别jvm的情况
    vmInitArgs.ignoreUnrecognized = JNI_TRUE;

    //设置启动类，注意分隔符为"/"
    const char startClass[] = "test/Test";
    //启动方法，一般是main函数，当然可以设置成其他函数
    const char startMethod[] = "main";

    //加载JVM,注意需要传入的字符串为LPCWSTR,指向一个常量Unicode字符串的32位指针，相当于const wchar_t*
    HINSTANCE jvmDLL = LoadLibrary(jvmPath);
    if (jvmDLL == NULL) {
        cout << "load jvm error" << endl;
        return false;
    }

    // 加载JVM启动方法 JNI_CreateJavaVM
    JNICREATEPROC createJavaVm = (JNICREATEPROC)GetProcAddress(jvmDLL, "JNI_CreateJavaVM");
    if (createJavaVm == NULL) {
        FreeLibrary(jvmDLL);
        cout << "load JNI_CreateJavaVM err" << endl;
        return false;
    }

    //创建JVM
    JNIEnv* env;
    JavaVM* jvm;
    jint jvmProc = (createJavaVm)(&jvm, (void**)&env, &vmInitArgs);
    if (jvmProc < 0 || jvm == NULL || env == NULL) {
        FreeLibrary(jvmDLL);
        cout << "create jvm error" << endl;
        return false;
    }


    //加载启动类
    jclass mainclass = env->FindClass(startClass);
    if (env->ExceptionCheck() == JNI_TRUE || mainclass == NULL) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        FreeLibrary(jvmDLL);
        cout << "load main class error" << endl;
        return false;
    }

    //加载启动方法
    jmethodID methedID = env->GetStaticMethodID(mainclass, startMethod, "([Ljava/lang/String;)V");
    if (env->ExceptionCheck() == JNI_TRUE || methedID == NULL) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        FreeLibrary(jvmDLL);
        cout << "load main method error" << endl;
        return false;
    }
    env->CallStaticVoidMethod(mainclass, methedID, NULL);
    //jvm释放
    jvm->DestroyJavaVM();
    return true;
}
