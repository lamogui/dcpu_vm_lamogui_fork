#include "lem1802.hpp"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <string>


namespace cpu {

namespace lem {

    const uint16_t Lem1802::def_font_map[128*2] = {     /// Default font map
#       include "lem1802_font.inc"
    };

    const uint16_t Lem1802::def_palette_map[16] = {     /// Default palette
#       include "lem1802_palette.inc"
    };

   // Clears the screen texture
   const static sf::Uint8 clear[Lem1802::WIDTH*Lem1802::HEIGHT*4]= {0};

    Lem1802::Lem1802() : screen_map (0), font_map (0), palette_map (0),
    border_col (0), ticks (0), enable (true), blink(0),screen(NULL)
    { 
    }

    Lem1802::~Lem1802() {
        if (screen) delete screen;
    }

    void Lem1802::attachTo (DCPU* cpu, size_t index) {
        this->IHardware::attachTo(cpu, index);
        tick_per_refresh = cpu->cpu_clock / Lem1802::FPS;
        initScreen();
        texture.create(Lem1802::WIDTH, Lem1802::HEIGHT);
        //texture.update(clear);
        texture.setRepeated(false);
        texture.setSmooth(false); //Pixel style
    }

    void Lem1802::handleInterrupt()
    {
        if (this->cpu == NULL)
            return;
        
        size_t s;
        switch (cpu->GetA() ) {
            case MEM_MAP_SCREEN:
                screen_map = cpu->GetB();

                if (screen_map != 0)
                    ticks = tick_per_refresh +1; // Force to do initial print
                break;

            case MEM_MAP_FONT:
                font_map = cpu->GetB();
                break;

            case MEM_MAP_PALETTE:
                palette_map = cpu->GetB();
                break;

            case SET_BORDER_COLOR:
                border_col = cpu->GetB() & 0xf;
                break;

            case MEM_DUMP_FONT:
                s = RAM_SIZE - 1 - cpu->GetB() < 256 ? 
                        RAM_SIZE - 1 - cpu->GetB() : 256 ;
                std::copy_n (Lem1802::def_font_map,s,cpu->getMem()+cpu->GetB());
                break;

            case MEM_DUMP_PALETTE:
                s = RAM_SIZE - 1 - cpu->GetB() < 16 ?
                        RAM_SIZE - 1 - cpu->GetB() : 16 ;
                std::copy_n (Lem1802::def_palette_map, s, 
                        cpu->getMem() + cpu->GetB() );
                break;

            default:
                // do nothing
                break;
        }
    }

    void Lem1802::tick()
    {
        if (++ticks > tick_per_refresh) {
            // Update screen at 60Hz aprox
            ticks = 0;
            this->show();
        }
        if (++blink > Lem1802::BLINKRATE*2)
            blink = 0;
    }

    void Lem1802::show()
    {
        //TODO use uint32_t to fast copy the color into the buffer
        //TODO remove all multiplications inside loop
        using namespace std;

        if (this->cpu == NULL || !need_render)
            return;
        need_render = false;
        uint8_t* pixel_pos = screen;
        //4 pixel per col 4 value per pixels
        const unsigned row_pixel_size = Lem1802::WIDTH*4; 
        const unsigned row_pixel_size_8 = row_pixel_size*8; 
        if (screen_map != 0 && enable) { // Update the texture
            for (unsigned row=0; row < Lem1802::ROWS; row++) {
                pixel_pos=screen + row*row_pixel_size_8;
                for (unsigned col=0; col < Lem1802::COLS; col++) {
                    uint16_t pos = screen_map + row * Lem1802::COLS + col;
                    unsigned char ascii = (unsigned char) (cpu->getMem()[pos] 
                                                            & 0x007F);
                    // Get palette indexes
                    uint16_t fg_ind = (cpu->getMem()[pos] & 0xF000) >> 12;
                    uint16_t bg_ind = (cpu->getMem()[pos] & 0x0F00) >> 8;
                    uint16_t fg_col, bg_col;

                    if (palette_map == 0) { // Use default palette
                        fg_col = Lem1802::def_palette_map[fg_ind &0xF];
                        bg_col = Lem1802::def_palette_map[bg_ind &0xF];
                    } else {
                        fg_col = cpu->getMem()[palette_map+ (fg_ind &0xF)];
                        bg_col = cpu->getMem()[palette_map+ (bg_ind &0xF)];
                    }
                    
                    // Does the blink
                    if (blink > BLINKRATE &&  
                           ((cpu->getMem()[pos] & 0x80) > 0) ) {
                        fg_col = bg_col;
                    }

                    // Composes RGBA values from palette colors
                    sf::Uint8 fg[] = {
                        (sf::Uint8)(((fg_col & 0xF00)>> 8) *0x11),
                        (sf::Uint8)(((fg_col & 0x0F0)>> 4) *0x11),
                        (sf::Uint8)( (fg_col & 0x00F)      *0x11),
                        0xFF };
                    sf::Uint8 bg[] = {
                        (sf::Uint8)(((bg_col & 0xF00)>> 8) *0x11),
                        (sf::Uint8)(((bg_col & 0x0F0)>> 4) *0x11),
                        (sf::Uint8)( (bg_col & 0x00F)      *0x11),
                        0xFF };
    
                    uint16_t glyph[2];
                    if (font_map == 0) { // Default font
                        glyph[0] = Lem1802::def_font_map[ascii*2]; 
                        glyph[1] = Lem1802::def_font_map[ascii*2+1];
                    } else {
                        glyph[0] = cpu->getMem()[font_map+ (ascii*2)]; 
                        glyph[1] = cpu->getMem()[font_map+ (ascii*2)+1]; 
                    }
                    uint8_t* current_pixel_pos = pixel_pos;
                    for (int i=8; i< 16; i++) { // Puts MSB of Words
                        // First word 
                        bool pixel = ((1<<i) & glyph[0]) > 0;
                        if (pixel) {
                            *(current_pixel_pos+0) = fg[0];
                            *(current_pixel_pos+1) = fg[1];
                            *(current_pixel_pos+2) = fg[2];
                            *(current_pixel_pos+3) = fg[3];
                        } else {
                            *(current_pixel_pos+0) = bg[0];
                            *(current_pixel_pos+1) = bg[1];
                            *(current_pixel_pos+2) = bg[2];
                            *(current_pixel_pos+3) = bg[3];
                        }
                        // Second word
                        pixel = ((1<<i) & glyph[1]) > 0;
                        if (pixel) {
                            *(current_pixel_pos+0+2*4) = fg[0];
                            *(current_pixel_pos+1+2*4) = fg[1];
                            *(current_pixel_pos+2+2*4) = fg[2];
                            *(current_pixel_pos+3+2*4) = fg[3];
                        } else {
                            *(current_pixel_pos+0+2*4) = bg[0];
                            *(current_pixel_pos+1+2*4) = bg[1];
                            *(current_pixel_pos+2+2*4) = bg[2];
                            *(current_pixel_pos+3+2*4) = bg[3];
                        }
                        current_pixel_pos += row_pixel_size;
                    }
                    current_pixel_pos = pixel_pos;

                    for (int i=0; i< 8; i++) { // Puts LSB of Words
                        // First word 
                        bool pixel = ((1<<i) & glyph[0]) >0;
                        if (pixel) {
                            *(current_pixel_pos+0+1*4) = fg[0];
                            *(current_pixel_pos+1+1*4) = fg[1];
                            *(current_pixel_pos+2+1*4) = fg[2];
                            *(current_pixel_pos+3+1*4) = fg[3];
                        } else {
                            *(current_pixel_pos+0+1*4) = bg[0];
                            *(current_pixel_pos+1+1*4) = bg[1];
                            *(current_pixel_pos+2+1*4) = bg[2];
                            *(current_pixel_pos+3+1*4) = bg[3];
                        }
                        // Secodn word
                        pixel = ((1<<i) & glyph[1]) > 0;
                        if (pixel) {
                            *(current_pixel_pos+0+3*4) = fg[0];
                            *(current_pixel_pos+1+3*4) = fg[1];
                            *(current_pixel_pos+2+3*4) = fg[2];
                            *(current_pixel_pos+3+3*4) = fg[3];
                        } else {
                            *(current_pixel_pos+0+3*4) = bg[0];
                            *(current_pixel_pos+1+3*4) = bg[1];
                            *(current_pixel_pos+2+3*4) = bg[2];
                            *(current_pixel_pos+3+3*4) = bg[3];
                        }
                        current_pixel_pos += row_pixel_size;
                    }
                    pixel_pos+=16;
                }
            }
            texture.update(screen);
        } else {
            texture.update(clear);
        }
    }
    
    sf::Color Lem1802::getBorder() const
    {
        uint16_t border;
        if (palette_map == 0) { // Use default palette
            border = Lem1802::def_palette_map[border_col];
        } else {
            border = cpu->getMem()[palette_map+ border_col];
        }
        return sf::Color(
                    ((border &0x0F00) >> 8) *0x11 ,
                    ((border &0x00F0) >> 4) *0x11 ,
                    ((border &0x000F) ) *0x11 ,
                    0xFF );
    }

    void Lem1802::setEnable(bool enable) 
    {
        this->enable = enable;
    }

} // END of NAMESPACE lem

} // END of NAMESPACE cpu
