# ProGuard rules for Lichii NX Explorer

# Keep JNI native methods
-keepclassmembers class com.eden.lichii.nxformat.NxFormatNative {
    private <methods>;
}

# Keep data classes
-keep class com.eden.lichii.nxformat.NxFormatNative$* { *; }
-keep class com.eden.lichii.nxformat.model.* { *; }

# Keep Compose related
-keep class androidx.compose.** { *; }
-keep class org.jetbrains.compose.** { *; }

# Keep Kotlin coroutines
-keep class kotlinx.coroutines.** { *; }

# Keep Gson
-keep class com.google.gson.** { *; }
-keepattributes Signature,EnclosingMethod

# Keep OkHttp/Retrofit if used
-keep class okhttp3.** { *; }
-keep class retrofit2.** { *; }

# Keep Zstandard
-keep class org.lz4.** { *; }