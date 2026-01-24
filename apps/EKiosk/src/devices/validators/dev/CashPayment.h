#ifdef Q_OS_WIN32
extern "C" {
int __declspec(dllexport) SendFirmWareDataByPath(int nPort, char *pFilePath);

int _stdcall GetDataStatus();

char *_stdcall GetFireVersion();

char *_stdcall GetFile1();
char *_stdcall GetFile2();
char *_stdcall GetFile3();
char *_stdcall GetFile4();
};
#endif // Q_OS_WIN32
