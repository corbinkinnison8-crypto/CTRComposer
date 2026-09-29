// ===================== FPS overlay =====================

static u64 fpsLastTick = 0;
static u32 fpsFrames = 0;
static u32 fpsValue = 0;

static u32 fpsLastSelect = 0;
static u64 fpsLastDraw = 0;

static void FpsOverlayInit(void)
{
    fpsLastTick = svcGetSystemTick();
    fpsLastSelect = REG32(LCD_TOP + LCD_SELECT) & 1;
    fpsFrames = 0;
    fpsValue = 0;
    fpsLastDraw = fpsLastTick;
}

static void FpsOverlayTick(void)
{
    u64 now = svcGetSystemTick();
    u32 sel = REG32(LCD_TOP + LCD_SELECT) & 1;

    /*
     * When the game changes the displayed top framebuffer,
     * count that as a presented frame.
     *
     * We deliberately do not count while CTRComposer itself
     * owns the screen.
     */
    if (sel != fpsLastSelect)
    {
        fpsFrames++;
        fpsLastSelect = sel;
    }

    /*
     * Update the reported FPS approximately once per second.
     * 3DS system ticks run at 268 MHz.
     */
    if (now - fpsLastTick >= 268000000ULL)
    {
        fpsValue = fpsFrames;
        fpsFrames = 0;
        fpsLastTick = now;
    }
}

static void FpsOverlayDraw(void)
{
    char buf[32];

    snprintf(buf, sizeof(buf), "FPS: %lu", (unsigned long)fpsValue);

    /*
     * Draw into CTRComposer's existing RGB888 compose buffer.
     * This gets presented normally by the engine.
     */
    CText(8, 8, buf, 255, 255, 255, 1);
}
