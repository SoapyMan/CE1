#if defined(_XBOX)

#ifdef __cplusplus
extern "C" void portable_makepath (
        char *path,
        const char *drive,
        const char *dir,
        const char *fname,
        const char *ext
		);
#else // _CPP
void portable_makepath (
        char *path,
        const char *drive,
        const char *dir,
        const char *fname,
        const char *ext
		);
#endif // _CPP

#elif defined(_WIN32)

#define portable_makepath _makepath 

#endif