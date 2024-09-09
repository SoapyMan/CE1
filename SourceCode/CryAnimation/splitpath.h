
#ifdef _XBOX
#ifdef __cplusplus
extern "C" void portable_splitpath (
        const char *path,
        char *drive,
        char *dir,
        char *fname,
        char *ext
        );
#else // __cplusplus
void portable_splitpath (
        const char *path,
        char *drive,
        char *dir,
        char *fname,
        char *ext
        );
#endif // __cplusplus
#elif defined(_WIN32)
#define portable_splitpath _splitpath
#endif