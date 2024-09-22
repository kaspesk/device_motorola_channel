package channel

import (
    "android/soong/android"
)

func init() {
    android.RegisterModuleType("motorola_channel_init_library_static", initLibraryFactory)
}
