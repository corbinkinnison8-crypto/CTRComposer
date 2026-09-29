// ===================== FPS overlay =====================

static u64 fpsWindowStart = 0;
static u32 fpsFrames = 0;
static u32 fpsValue = 0;
static u32 fpsLastSelect = 0;
static int fpsReady = 0;

static void FpsOverlayInit(void)
{
    fpsWindowStart = svcGetSystemTick();
    fpsFrames = 0;
    fpsValue = 0;
    fpsLastSelect = REG32(LCD_TOP + LCD_SELECT) & 1;
    fpsReady = 1;
}

static void FpsOverlayTick(void)
{
    if (!fpsReady)
        FpsOverlayInit();

    /*
     * CTRComposer owns LCD_SELECT while its menu is open.
     * Do not count those flips as game frames.
     */
    if (g_lcdSelValid)
    {
        fpsLastSelect = REG32(LCD_TOP + LCD_SELECT) & 1;
        fpsFrames = 0;
        fpsWindowStart = svcGetSystemTick();
        return;
    }

    u32 select = REG32(LCD_TOP + LCD_SELECT) & 1;

    /*
     * Outside the plugin menu, a change of LCD_SELECT means
     * the game selected the other top-screen framebuffer.
     */
    if (select != fpsLastSelect)
    {
        fpsFrames++;
        fpsLastSelect = select;
    }

    u64 now = svcGetSystemTick();

    /*
     * Update the displayed FPS once per second.
     * 3DS system tick frequency is 268 MHz.
     */
    if (now - fpsWindowStart >= 268000000ULL)
    {
        fpsValue = fpsFrames;
        fpsFrames = 0;
        fpsWindowStart = now;
    }
}

static void FpsOverlayDraw(void)
{
    if (!fpsReady || g_lcdSelValid)
        return;

    char text[32];

    snprintf(
        text,
        sizeof(text),
        "FPS: %lu",
        (unsigned long)fpsValue
    );

    /*
     * We don't need GrabFb().
     *
     * The FPS box is deliberately opaque, so we can build
     * only this small rectangle in the compose buffer.
     */
    CFill(4, 4, 82, 22, 0, 0, 0);
    CText(8, 7, text, 255, 255, 255, 1);

    /*
     * Stamp the small box directly onto the game's currently
     * visible framebuffer.
     *
     * The game can overwrite it on its next frame, so the
     * main loop calls this every 4 ms.
     */
    FbInfo f = GetFb(0);
    BlitTopRect(&f, 4, 4, 82, 22);
}
