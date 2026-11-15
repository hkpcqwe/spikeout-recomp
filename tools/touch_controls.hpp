// On-screen touch controls for phones/tablets (iOS): a virtual stick and the
// cabinet's six buttons (coin, start, beat, charge, jump, shift), plus a
// menu button. Fingers and mouse (Simulator) map to tools::Action; draw with
// SDL's 2D renderer, or paint into an RGBA buffer for the GPU overlay.
#pragma once

#include "settings.hpp"

#include <SDL3/SDL.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

namespace tools {

class TouchControls {
public:
    enum class Spot : int {
        Stick = 0,
        Coin,
        Start,
        Beat,
        Charge,
        Jump,
        Shift,
        Menu,
        Count
    };

    struct Button {
        float x = 0, y = 0, r = 0;
        const char* label = "";
        Spot spot = Spot::Stick;
    };

    struct Layout {
        float stick_x = 0, stick_y = 0, stick_r = 0;
        float knob_x = 0, knob_y = 0;
        std::array<Button, 6> buttons{};
        Button menu{};
        int width = 0, height = 0;
    };

    void set_enabled(bool on) { enabled_ = on; }
    bool enabled() const { return enabled_; }
    // When the Esc menu is open: keep only the MENU button (to close it) and
    // let every other tap fall through to the menu's own click handling.
    void set_menu_chrome_only(bool on) {
        if (chrome_only_ == on) return;
        chrome_only_ = on;
        dirty_ = true;
        if (on) {
            for (auto& f : fingers_) f.used = false;
            held_.fill(false);
            layout_.knob_x = layout_.stick_x;
            layout_.knob_y = layout_.stick_y;
        }
    }
    bool menu_chrome_only() const { return chrome_only_; }
    bool menu_pressed() const { return menu_edge_; }
    void clear_menu_edge() { menu_edge_ = false; }
    std::array<bool, kActions> held() const { return held_; }
    bool dirty() const { return dirty_; }
    void clear_dirty() { dirty_ = false; }
    const Layout& layout() const { return layout_; }

    void layout_for(int width, int height) {
        if (width == layout_.width && height == layout_.height) return;
        layout_.width = width;
        layout_.height = height;
        const float short_side = static_cast<float>(std::min(width, height));
        const float pad = short_side * 0.04f;
        layout_.stick_r = short_side * 0.18f;
        layout_.stick_x = pad + layout_.stick_r;
        layout_.stick_y = static_cast<float>(height) - pad - layout_.stick_r;
        layout_.knob_x = layout_.stick_x;
        layout_.knob_y = layout_.stick_y;

        const float br = short_side * 0.075f;
        const float right = static_cast<float>(width) - pad - br * 2.6f;
        const float mid_y = static_cast<float>(height) - pad - br * 2.6f;
        auto set = [&](std::size_t i, Spot s, const char* label, float x, float y) {
            layout_.buttons[i] = {x, y, br, label, s};
        };
        set(0, Spot::Coin, "COIN", pad + br, pad + br);
        set(1, Spot::Start, "START", static_cast<float>(width) - pad - br, pad + br);
        set(2, Spot::Shift, "SH", right + br * 1.3f, mid_y - br * 1.5f);
        set(3, Spot::Charge, "CH", right, mid_y);
        set(4, Spot::Jump, "JP", right + br * 2.6f, mid_y);
        set(5, Spot::Beat, "AT", right + br * 1.3f, mid_y + br * 1.5f);
        layout_.menu = {static_cast<float>(width) * 0.5f, pad + br * 0.85f, br * 0.85f, "MENU", Spot::Menu};
        dirty_ = true;
    }

    bool handle_event(const SDL_Event& e) {
        if (!enabled_ || layout_.width <= 0) return false;
        switch (e.type) {
        case SDL_EVENT_FINGER_DOWN:
            return press(e.tfinger.fingerID, e.tfinger.x * static_cast<float>(layout_.width),
                         e.tfinger.y * static_cast<float>(layout_.height));
        case SDL_EVENT_FINGER_MOTION:
            return move(e.tfinger.fingerID, e.tfinger.x * static_cast<float>(layout_.width),
                        e.tfinger.y * static_cast<float>(layout_.height));
        case SDL_EVENT_FINGER_UP:
            return release(e.tfinger.fingerID);
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (e.button.button != SDL_BUTTON_LEFT) return false;
            return press(kMouseId, e.button.x, e.button.y);
        case SDL_EVENT_MOUSE_MOTION:
            if (!(e.motion.state & SDL_BUTTON_LMASK)) return false;
            return move(kMouseId, e.motion.x, e.motion.y);
        case SDL_EVENT_MOUSE_BUTTON_UP:
            if (e.button.button != SDL_BUTTON_LEFT) return false;
            return release(kMouseId);
        default:
            return false;
        }
    }

    void draw_sdl(SDL_Renderer* r) const {
        if (!enabled_ || !r || layout_.width <= 0) return;
        auto fill_disk = [&](float cx, float cy, float rad, Uint8 a, Uint8 red, Uint8 g, Uint8 b) {
            SDL_SetRenderDrawColor(r, red, g, b, a);
            for (float y = -rad; y <= rad; y += 1.0f) {
                const float dx = std::sqrt(std::max(0.0f, rad * rad - y * y));
                SDL_RenderLine(r, cx - dx, cy + y, cx + dx, cy + y);
            }
        };
        auto stroke_disk = [&](float cx, float cy, float rad, Uint8 a, Uint8 red, Uint8 g, Uint8 b) {
            SDL_SetRenderDrawColor(r, red, g, b, a);
            const int n = std::max(28, static_cast<int>(rad));
            float px = cx + rad, py = cy;
            for (int i = 1; i <= n; ++i) {
                const float ang = 6.2831853f * static_cast<float>(i) / static_cast<float>(n);
                const float x = cx + std::cos(ang) * rad, y = cy + std::sin(ang) * rad;
                SDL_RenderLine(r, px, py, x, y);
                px = x;
                py = y;
            }
        };
        auto label = [&](float cx, float cy, const char* text) {
            if (!text || !*text) return;
            const float tw = static_cast<float>(std::strlen(text)) * SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;
            SDL_SetRenderDrawColor(r, 255, 255, 255, 230);
            SDL_RenderDebugText(r, cx - tw * 0.5f, cy - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * 0.5f, text);
        };

        if (!chrome_only_) {
            const bool stick_on = spot_held(Spot::Stick);
            fill_disk(layout_.stick_x, layout_.stick_y, layout_.stick_r, stick_on ? 90 : 55, 200, 200, 210);
            stroke_disk(layout_.stick_x, layout_.stick_y, layout_.stick_r, 160, 255, 255, 255);
            fill_disk(layout_.knob_x, layout_.knob_y, layout_.stick_r * 0.38f, stick_on ? 200 : 140, 255, 211, 74);

            for (const auto& b : layout_.buttons) {
                const bool on = spot_held(b.spot);
                fill_disk(b.x, b.y, b.r, on ? 160 : 70, on ? 255 : 200, on ? 211 : 200, on ? 74 : 210);
                stroke_disk(b.x, b.y, b.r, 180, 255, 255, 255);
                label(b.x, b.y, b.label);
            }
        }
        {
            const bool on = spot_held(Spot::Menu);
            fill_disk(layout_.menu.x, layout_.menu.y, layout_.menu.r, on ? 160 : 70, 100, 160, 255);
            stroke_disk(layout_.menu.x, layout_.menu.y, layout_.menu.r, 180, 255, 255, 255);
            label(layout_.menu.x, layout_.menu.y, layout_.menu.label);
        }
    }

    // 0xAARRGGBB overlay for the GPU path (0 where unused).
    void paint_rgba(std::vector<uint32_t>& out, int width, int height) const {
        out.assign(static_cast<std::size_t>(width) * static_cast<std::size_t>(height), 0);
        if (!enabled_ || width <= 0 || height <= 0) return;
        auto put = [&](int x, int y, uint32_t c) {
            if (x < 0 || y < 0 || x >= width || y >= height) return;
            uint32_t& d = out[static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)];
            const uint32_t sa = c >> 24, sr = (c >> 16) & 255, sg = (c >> 8) & 255, sb = c & 255;
            const uint32_t da = d >> 24, dr = (d >> 16) & 255, dg = (d >> 8) & 255, db = d & 255;
            const uint32_t out_a = sa + da * (255 - sa) / 255;
            if (!out_a) {
                d = 0;
                return;
            }
            d = (out_a << 24) | (((sr * sa + dr * da * (255 - sa) / 255) / out_a) << 16) |
                (((sg * sa + dg * da * (255 - sa) / 255) / out_a) << 8) |
                ((sb * sa + db * da * (255 - sa) / 255) / out_a);
        };
        auto disk = [&](float cx, float cy, float rad, uint32_t c) {
            const int x0 = static_cast<int>(cx - rad), x1 = static_cast<int>(cx + rad);
            const int y0 = static_cast<int>(cy - rad), y1 = static_cast<int>(cy + rad);
            const float r2 = rad * rad;
            for (int y = y0; y <= y1; ++y)
                for (int x = x0; x <= x1; ++x) {
                    const float dx = static_cast<float>(x) + 0.5f - cx, dy = static_cast<float>(y) + 0.5f - cy;
                    if (dx * dx + dy * dy <= r2) put(x, y, c);
                }
        };
        auto ring = [&](float cx, float cy, float rad, uint32_t c) {
            const float r2 = rad * rad, r1 = std::max(0.0f, rad - 2) * std::max(0.0f, rad - 2);
            const int x0 = static_cast<int>(cx - rad - 1), x1 = static_cast<int>(cx + rad + 1);
            const int y0 = static_cast<int>(cy - rad - 1), y1 = static_cast<int>(cy + rad + 1);
            for (int y = y0; y <= y1; ++y)
                for (int x = x0; x <= x1; ++x) {
                    const float dx = static_cast<float>(x) + 0.5f - cx, dy = static_cast<float>(y) + 0.5f - cy;
                    const float d = dx * dx + dy * dy;
                    if (d <= r2 && d >= r1) put(x, y, c);
                }
        };

        if (!chrome_only_) {
            const bool stick_on = spot_held(Spot::Stick);
            disk(layout_.stick_x, layout_.stick_y, layout_.stick_r, stick_on ? 0x5AC8CCD4u : 0x38C8CCD4u);
            ring(layout_.stick_x, layout_.stick_y, layout_.stick_r, 0xA0FFFFFFu);
            disk(layout_.knob_x, layout_.knob_y, layout_.stick_r * 0.38f, stick_on ? 0xC8FFD34Au : 0x8CFFD34Au);
            for (const auto& b : layout_.buttons) {
                const bool on = spot_held(b.spot);
                disk(b.x, b.y, b.r, on ? 0xA0FFD34Au : 0x46C8CCD4u);
                ring(b.x, b.y, b.r, 0xB4FFFFFFu);
            }
        }
        {
            const bool on = spot_held(Spot::Menu);
            disk(layout_.menu.x, layout_.menu.y, layout_.menu.r, on ? 0xA064A0FFu : 0x4664A0FFu);
            ring(layout_.menu.x, layout_.menu.y, layout_.menu.r, 0xB4FFFFFFu);
        }
    }

private:
    static constexpr SDL_FingerID kMouseId = static_cast<SDL_FingerID>(-1);

    struct Finger {
        SDL_FingerID id = 0;
        Spot spot = Spot::Count;
        float x = 0, y = 0;
        bool used = false;
    };

    bool enabled_ = false;
    bool chrome_only_ = false;
    bool menu_edge_ = false;
    bool dirty_ = true;
    Layout layout_;
    std::array<bool, kActions> held_{};
    std::array<Finger, 10> fingers_{};

    Finger* find(SDL_FingerID id) {
        for (auto& f : fingers_)
            if (f.used && f.id == id) return &f;
        return nullptr;
    }
    bool spot_held(Spot s) const {
        for (const auto& f : fingers_)
            if (f.used && f.spot == s) return true;
        return false;
    }

    Spot hit(float x, float y) const {
        auto in = [&](float cx, float cy, float r) {
            const float dx = x - cx, dy = y - cy;
            return dx * dx + dy * dy <= r * r;
        };
        if (in(layout_.menu.x, layout_.menu.y, layout_.menu.r * 1.15f)) return Spot::Menu;
        if (chrome_only_) return Spot::Count;
        for (const auto& b : layout_.buttons)
            if (in(b.x, b.y, b.r * 1.25f)) return b.spot;
        if (in(layout_.stick_x, layout_.stick_y, layout_.stick_r * 1.4f)) return Spot::Stick;
        return Spot::Count;
    }

    bool press(SDL_FingerID id, float x, float y) {
        const Spot s = hit(x, y);
        if (s == Spot::Count) return false;
        Finger* slot = find(id);
        if (!slot) {
            for (auto& f : fingers_)
                if (!f.used) {
                    slot = &f;
                    break;
                }
        }
        if (!slot) return false;
        slot->used = true;
        slot->id = id;
        slot->spot = s;
        slot->x = x;
        slot->y = y;
        if (s == Spot::Menu) menu_edge_ = true;
        if (s == Spot::Stick) update_stick(*slot);
        dirty_ = true;
        recompute();
        return true;
    }

    bool move(SDL_FingerID id, float x, float y) {
        Finger* f = find(id);
        if (!f) return false;
        f->x = x;
        f->y = y;
        if (f->spot == Spot::Stick) update_stick(*f);
        dirty_ = true;
        recompute();
        return true;
    }

    bool release(SDL_FingerID id) {
        Finger* f = find(id);
        if (!f) return false;
        f->used = false;
        dirty_ = true;
        recompute();
        return true;
    }

    void update_stick(Finger& f) {
        float dx = f.x - layout_.stick_x, dy = f.y - layout_.stick_y;
        const float len = std::sqrt(dx * dx + dy * dy);
        const float max_r = layout_.stick_r * 0.72f;
        if (len > max_r && len > 0.001f) {
            dx *= max_r / len;
            dy *= max_r / len;
        }
        layout_.knob_x = layout_.stick_x + dx;
        layout_.knob_y = layout_.stick_y + dy;
    }

    void recompute() {
        held_.fill(false);
        bool stick = false;
        for (const auto& f : fingers_) {
            if (!f.used) continue;
            switch (f.spot) {
            case Spot::Coin: held_[static_cast<std::size_t>(Action::Coin)] = true; break;
            case Spot::Start: held_[static_cast<std::size_t>(Action::Start)] = true; break;
            case Spot::Beat: held_[static_cast<std::size_t>(Action::Beat)] = true; break;
            case Spot::Charge: held_[static_cast<std::size_t>(Action::Charge)] = true; break;
            case Spot::Jump: held_[static_cast<std::size_t>(Action::Jump)] = true; break;
            case Spot::Shift: held_[static_cast<std::size_t>(Action::Shift)] = true; break;
            case Spot::Stick: stick = true; break;
            default: break;
            }
        }
        if (!stick) {
            layout_.knob_x = layout_.stick_x;
            layout_.knob_y = layout_.stick_y;
            return;
        }
        const float sx = layout_.knob_x - layout_.stick_x, sy = layout_.knob_y - layout_.stick_y;
        const float dead = layout_.stick_r * 0.22f;
        if (std::fabs(sy) >= dead) {
            if (sy < 0) held_[static_cast<std::size_t>(Action::Up)] = true;
            if (sy > 0) held_[static_cast<std::size_t>(Action::Down)] = true;
        }
        if (std::fabs(sx) >= dead) {
            if (sx < 0) held_[static_cast<std::size_t>(Action::Left)] = true;
            if (sx > 0) held_[static_cast<std::size_t>(Action::Right)] = true;
        }
    }
};

} // namespace tools
