#include <emscripten/bind.h>
#include <emscripten.h>
using namespace emscripten;

extern uintptr_t wasm_updated_screen;

EMSCRIPTEN_BINDINGS(db48x_module)
// ----------------------------------------------------------------------------
//   Bindings for the DB48X JavaScript WASM module
// ----------------------------------------------------------------------------
{
    function("ui_battery", &ui_battery);
    function("ui_lcd_buffer", &ui_lcd_buffer);
    function("ui_init", &ui_init);
    function("ui_push_key", &key_push);
}
