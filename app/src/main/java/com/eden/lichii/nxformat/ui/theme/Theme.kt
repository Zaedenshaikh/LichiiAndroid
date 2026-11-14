package com.eden.lichii.nxformat.ui.theme

import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color

private val DarkColorScheme = darkColorScheme(
    primary = Color(0xFFBB86FC),
    primaryContainer = Color(0xFF3700B3),
    secondary = Color(0xFF03DAC6),
    secondaryContainer = Color(0xFF006064),
    tertiary = Color(0xFFCF6679),
    background = Color(0xFF121212),
    surface = Color(0xFF1E1E1E),
    surfaceContainerLow = Color(0xFF2C2C2C),
    surfaceContainer = Color(0xFF383838),
    surfaceContainerHigh = Color(0xFF484848),
    surfaceContainerHighest = Color(0xFF585858),
    onPrimary = Color(0xFF000000),
    onPrimaryContainer = Color(0xFFEADDFF),
    onSecondary = Color(0xFF000000),
    onSecondaryContainer = Color(0xFFA7F0F5),
    onTertiary = Color(0xFF000000),
    onBackground = Color(0xFFFFFFFF),
    onSurface = Color(0xFFFFFFFF),
    onSurfaceVariant = Color(0xFFCAC4D0),
    outline = Color(0xFF79747E),
    outlineVariant = Color(0xFF49454F),
    inverseSurface = Color(0xFFE6E1E5),
    inverseOnSurface = Color(0xFF313033),
    shadow = Color(0xFF000000),
    scrim = Color(0xFF000000),
    surfaceTint = Color(0xFFBB86FC)
)

private val LightColorScheme = lightColorScheme(
    primary = Color(0xFF6200EE),
    primaryContainer = Color(0xFFEADDFF),
    secondary = Color(0xFF018786),
    secondaryContainer = Color(0xFFA7F0F5),
    tertiary = Color(0xFF7B61FF),
    background = Color(0xFFF5F5F5),
    surface = Color(0xFFFFFFFF),
    surfaceContainerLow = Color(0xFFF2F2F2),
    surfaceContainer = Color(0xFFE8E8E8),
    surfaceContainerHigh = Color(0xFFDEDEDE),
    surfaceContainerHighest = Color(0xFFD4D4D4),
    onPrimary = Color(0xFFFFFFFF),
    onPrimaryContainer = Color(0xFF21005D),
    onSecondary = Color(0xFFFFFFFF),
    onSecondaryContainer = Color(0xFF001F1F),
    onTertiary = Color(0xFFFFFFFF),
    onBackground = Color(0xFF1C1B1F),
    onSurface = Color(0xFF1C1B1F),
    onSurfaceVariant = Color(0xFF49454F),
    outline = Color(0xFF79747E),
    outlineVariant = Color(0xFFCAC4D0),
    inverseSurface = Color(0xFF313033),
    inverseOnSurface = Color(0xFFF5F0F4),
    shadow = Color(0xFF000000),
    scrim = Color(0xFF000000),
    surfaceTint = Color(0xFF6200EE)
)

@Composable
fun LichiiAndroidTheme(
    darkTheme: Boolean = false,
    content: @Composable () -> Unit
) {
    val colorScheme = if (darkTheme) DarkColorScheme else LightColorScheme
    MaterialTheme(
        colorScheme = colorScheme,
        typography = Typography,
        content = content
    )
}

val Typography = androidx.compose.material3.Typography(
    displayLarge = androidx.compose.material3.TextStyle(
        fontFamily = androidx.compose.ui.text.font.FontFamily.Default,
        fontWeight = androidx.compose.ui.text.font.FontWeight.Normal,
        fontSize = 57.sp,
        lineHeight = androidx.compose.ui.unit.sp(64),
        letterSpacing = androidx.compose.ui.unit.sp(-0.25)
    ),
    displayMedium = androidx.compose.material3.TextStyle(
        fontFamily = androidx.compose.ui.text.font.FontFamily.Default,
        fontWeight = androidx.compose.ui.text.font.FontWeight.Normal,
        fontSize = 45.sp,
        lineHeight = androidx.compose.ui.unit.sp(52),
        letterSpacing = androidx.compose.ui.unit.sp(0)
    ),
    displaySmall = androidx.compose.material3.TextStyle(
        fontFamily = androidx.compose.ui.text.font.FontFamily.Default,
        fontWeight = androidx.compose.ui.text.font.FontWeight.Normal,
        fontSize = 36.sp,
        lineHeight = androidx.compose.ui.unit.sp(44),
        letterSpacing = androidx.compose.ui.unit.sp(0)
    ),
    headlineLarge = androidx.compose.material3.TextStyle(
        fontFamily = androidx.compose.ui.text.font.FontFamily.Default,
        fontWeight = androidx.compose.ui.text.font.FontWeight.Normal,
        fontSize = 32.sp,
        lineHeight = androidx.compose.ui.unit.sp(40),
        letterSpacing = androidx.compose.ui.unit.sp(0)
    ),
    headlineMedium = androidx.compose.material3.TextStyle(
        fontFamily = androidx.compose.ui.text.font.FontFamily.Default,
        fontWeight = androidx.compose.ui.text.font.FontWeight.Normal,
        fontSize = 28.sp,
        lineHeight = androidx.compose.ui.unit.sp(36),
        letterSpacing = androidx.compose.ui.unit.sp(0)
    ),
    headlineSmall = androidx.compose.material3.TextStyle(
        fontFamily = androidx.compose.ui.text.font.FontFamily.Default,
        fontWeight = androidx.compose.ui.text.font.FontWeight.Normal,
        fontSize = 24.sp,
        lineHeight = androidx.compose.ui.unit.sp(32),
        letterSpacing = androidx.compose.ui.unit.sp(0)
    ),
    titleLarge = androidx.compose.material3.TextStyle(
        fontFamily = androidx.compose.ui.text.font.FontFamily.Default,
        fontWeight = androidx.compose.ui.text.font.FontWeight.Medium,
        fontSize = 22.sp,
        lineHeight = androidx.compose.ui.unit.sp(28),
        letterSpacing = androidx.compose.ui.unit.sp(0)
    ),
    titleMedium = androidx.compose.material3.TextStyle(
        fontFamily = androidx.compose.ui.text.font.FontFamily.Default,
        fontWeight = androidx.compose.ui.text.font.FontWeight.Medium,
        fontSize = 16.sp,
        lineHeight = androidx.compose.ui.unit.sp(24),
        letterSpacing = androidx.compose.ui.unit.sp(0.15)
    ),
    titleSmall = androidx.compose.material3.TextStyle(
        fontFamily = androidx.compose.ui.text.font.FontFamily.Default,
        fontWeight = androidx.compose.ui.text.font.FontWeight.Medium,
        fontSize = 14.sp,
        lineHeight = androidx.compose.ui.unit.sp(20),
        letterSpacing = androidx.compose.ui.unit.sp(0.1)
    ),
    bodyLarge = androidx.compose.material3.TextStyle(
        fontFamily = androidx.compose.ui.text.font.FontFamily.Default,
        fontWeight = androidx.compose.ui.text.font.FontWeight.Normal,
        fontSize = 16.sp,
        lineHeight = androidx.compose.ui.unit.sp(24),
        letterSpacing = androidx.compose.ui.unit.sp(0.5)
    ),
    bodyMedium = androidx.compose.material3.TextStyle(
        fontFamily = androidx.compose.ui.text.font.FontFamily.Default,
        fontWeight = androidx.compose.ui.text.font.FontWeight.Normal,
        fontSize = 14.sp,
        lineHeight = androidx.compose.ui.unit.sp(20),
        letterSpacing = androidx.compose.ui.unit.sp(0.25)
    ),
    bodySmall = androidx.compose.material3.TextStyle(
        fontFamily = androidx.compose.ui.text.font.FontFamily.Default,
        fontWeight = androidx.compose.ui.text.font.FontWeight.Normal,
        fontSize = 12.sp,
        lineHeight = androidx.compose.ui.unit.sp(16),
        letterSpacing = androidx.compose.ui.unit.sp(0.4)
    ),
    labelLarge = androidx.compose.material3.TextStyle(
        fontFamily = androidx.compose.ui.text.font.FontFamily.Default,
        fontWeight = androidx.compose.ui.text.font.FontWeight.Medium,
        fontSize = 14.sp,
        lineHeight = androidx.compose.ui.unit.sp(20),
        letterSpacing = androidx.compose.ui.unit.sp(0.1)
    ),
    labelMedium = androidx.compose.material3.TextStyle(
        fontFamily = androidx.compose.ui.text.font.FontFamily.Default,
        fontWeight = androidx.compose.ui.text.font.FontWeight.Medium,
        fontSize = 12.sp,
        lineHeight = androidx.compose.ui.unit.sp(16),
        letterSpacing = androidx.compose.ui.unit.sp(0.5)
    ),
    labelSmall = androidx.compose.material3.TextStyle(
        fontFamily = androidx.compose.ui.text.font.FontFamily.Default,
        fontWeight = androidx.compose.ui.text.font.FontWeight.Medium,
        fontSize = 11.sp,
        lineHeight = androidx.compose.ui.unit.sp(16),
        letterSpacing = androidx.compose.ui.unit.sp(0.5)
    )
)