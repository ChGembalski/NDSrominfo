/***************************************************************************
 *   Copyright (C) 2013 by Christoph Gembalski   *
 *   christoph.gembalski@web.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef NDS_ROMINFO_H
#define NDS_ROMINFO_H

#include <iostream>
#include <endian.h>
#include <stdint.h>


// determine byte order.  nds rom files are little endian
#if __BYTE_ORDER == __LITTLE_ENDIAN
#  define endian_16(a) a
#  define endian_32(a) a
#else // __BYTE_ORDER == __BIG_ENDIAN
#  define endian_16(a) ((((uint16_t)(a) & 0xff00) >> 8) | \
                        (((uint16_t)(a) & 0x00ff) << 8))
#  define endian_32(a) ((((uint32_t)(a) & 0xff000000) >> 24) | \
                        (((uint32_t)(a) & 0x00ff0000) >> 8)  | \
                        (((uint32_t)(a) & 0x0000ff00) << 8)  | \
                        (((uint32_t)(a) & 0x000000ff) << 24))
#endif

typedef struct __attribute__ ((packed)) {
    uint8_t  game_title[12];// 000h    12     Game Title  (Uppercase ASCII, padded with 00h)
    uint8_t  game_code[4];  // 00Ch    4      Gamecode    (Uppercase ASCII, NTR-<code>)        (0=homebrew)
    uint8_t  maker_code[2]; // 010h    2      Makercode   (Uppercase ASCII, eg. "01"=Nintendo) (0=homebrew)
    uint8_t  unit_code;     // 012h    1      Unitcode    (00h=Nintendo DS)
    uint8_t  enc_seed_sel;  // 013h    1      Encryption Seed Select (00..07h, usually 00h)
    uint8_t  dev_capacity;  // 014h    1      Devicecapacity         (Chipsize = 128KB SHL nn) (eg. 7 = 16MB)
    uint8_t  reserve1[9];   // 015h    9      Reserved           (zero filled)
    uint8_t  rom_ver;       // 01Eh    1      ROM Version        (usually 00h)
    uint8_t  auto_start;    // 01Fh    1      Autostart (Bit2: Skip "Press Button" after Health and Safety)
                            // (Also skips bootmenu, even in Manual mode & even Start pressed)
    uint32_t rom_offset9;   // 020h    4      ARM9 rom_offset    (4000h and up, align 1000h)
    uint32_t ent_addr9;     // 024h    4      ARM9 entry_address (2000000h..23BFE00h)
    uint32_t ram_addr9;     // 028h    4      ARM9 ram_address   (2000000h..23BFE00h)
    uint32_t size9;         // 02Ch    4      ARM9 size          (max 3BFE00h) (3839.5KB)
    uint32_t rom_offset4;   // 030h    4      ARM7 rom_offset    (8000h and up)
    uint32_t ent_addr7;     // 034h    4      ARM7 entry_address (2000000h..23BFE00h, or 37F8000h..3807E00h)
    uint32_t ram_addr7;     // 038h    4      ARM7 ram_address   (2000000h..23BFE00h, or 37F8000h..3807E00h)
    uint32_t size_7;        // 03Ch    4      ARM7 size          (max 3BFE00h, or FE00h) (3839.5KB, 63.5KB)
    uint32_t fnt_off;       // 040h    4      File Name Table (FNT) offset
    uint32_t fnt_size;      // 044h    4      File Name Table (FNT) size
    uint32_t fat_off;       // 048h    4      File Allocation Table (FAT) offset
    uint32_t fat_size;      // 04Ch    4      File Allocation Table (FAT) size
    uint32_t overlay_off9;  // 050h    4      File ARM9 overlay_offset
    uint32_t overlay_size9; // 054h    4      File ARM9 overlay_size
    uint32_t overlay_off7;  // 058h    4      File ARM7 overlay_offset
    uint32_t overlay_size7; // 05Ch    4      File ARM7 overlay_size
    uint32_t normal_cmd;    // 060h    4      Port 40001A4h setting for normal commands (usually 00586000h)
    uint32_t key_cmd;       // 064h    4      Port 40001A4h setting for KEY1 commands   (usually 001808F8h)
    uint32_t icon_title_off;// 068h    4      Icon_title_offset (0=None) (8000h and up)
    uint16_t sec_sum;       // 06Ch    2      Secure Area Checksum, CRC-16 of [ [20h]..7FFFh]
    uint16_t sec_timeout;   // 06Eh    2      Secure Area Loading Timeout (usually 051Eh)
    uint32_t load_list9;    // 070h    4      ARM9 Auto Load List RAM Address (?)
    uint32_t load_list7;    // 074h    4      ARM7 Auto Load List RAM Address (?)
    uint8_t  sec_dis[8];    // 078h    8      Secure Area Disable (by encrypted "NmMdOnly") (usually zero)
    uint32_t rom_size;      // 080h    4      Total Used ROM size (remaining/unused bytes usually FFh-padded)
    uint32_t rom_hdr_size;  // 084h    4      ROM Header Size (4000h)
    uint8_t  reserve2[0x38];// 088h    38h    Reserved (zero filled)
    uint8_t  reserve3[0x9c];// 0C0h    9Ch    Nintendo Logo (compressed bitmap, same as in GBA Headers)
    uint16_t logo_sum;      // 15Ch    2      Nintendo Logo Checksum, CRC-16 of [0C0h-15Bh], fixed CF56h
    uint16_t hdr_sum;       // 15Eh    2      Header Checksum, CRC-16 of [000h-15Dh]
    uint32_t dbg_rom_off;   // 160h    4      Debug rom_offset   (0=none) (8000h and up)       ;only if debug
    uint32_t dbg_size;      // 164h    4      Debug size         (0=none) (max 3BFE00h)        ;version with
    uint32_t dbg_ram_addr;  // 168h    4      Debug ram_address  (0=none) (2400000h..27BFE00h) ;SIO and 8MB
    uint32_t dbg_reserve[5];// 16Ch    20     Debug reserved
    uint8_t  cfgsetting[52];// 180h    52     Config settings
    uint32_t access_ctrl;   // 1B4h    4      Access Control 
    uint32_t arm7_scfg_mask;// 1B8h    4      ARM7 SCFG EXT mask (controls which devices to enable) 
    uint32_t reserve4;      // 1BCh    4      Reserved/flags? When bit2 of byte 0x1bf is set, usage of banner.sav 
                            //                from the title data dir is enabled.(additional banner data) 
    uint32_t rom2_offset9;  // 1C0h    4      ARM9i rom offset 
    uint32_t entry_address9;// 1C4h    4      Reserved 
    uint32_t ram_address9;  // 1CBh    4      ARM9i load address 
    uint32_t rom2_size9;    // 1CCh    4      ARM9i size 
    uint32_t rom2_offset7;  // 1D0h    4      ARM7i rom offset 
    uint32_t entry_address7;// 1D4h    4      Pointer to base address where various structures and parameters are passed to the title 
    uint32_t ram_address7;  // 1D8h    4      ARM7i load address 
    uint32_t rom2_size7;    // 1DCh    4      ARM7i size 
    uint32_t dntr_reg_ofs;  // 1E0h    4      Digest NTR region offset 
    uint32_t dntr_reg_len;  // 1E4h    4      Digest NTR region length 
    uint32_t dtwl_reg_ofs;  // 1E8h    4      Digest TWL region offset 
    uint32_t dtwl_reg_len;  // 1ECh    4      Digest TWL region length 
    uint32_t hash1_start;   // 1F0h    4      Digest Sector Hashtable offset 
    uint32_t hash1_size;    // 1F4h    4      Digest Sector Hashtable length 
    uint32_t hash2_start;   // 1F8h    4      Digest Block Hashtable offset 
    uint32_t hash2_size;    // 1FCh    4      Digest Block Hashtable length 
    uint32_t dig_sec_size;  // 200h    4      Digest Sector size 
    uint32_t dig_block_cnt; // 204h    4      Digest Block sectorcount 
    uint8_t  reserve5[24];  // 208h    24     Reserved
    uint32_t mcrypt1_ofs;   // 220h    4      Modcrypt area 1 offset 
    uint32_t mcrypt1_size;  // 224h    4      Modcrypt area 1 size 
    uint32_t mcrypt2_ofs;   // 228h    4      Modcrypt area 2 offset 
    uint32_t mcrypt2_size;  // 22Ch    4      Modcrypt area 2 size 
    uint32_t title_id[2];   // 230h    8      Title ID
    uint8_t  reserve6[200]; // 238h    200    Reserved
    uint32_t hmac_hash9e[5];// 300h    20     ARM9 (with encrypted secure area) SHA1 HMAC hash 
    uint32_t hmac_hash7[5]; // 314h    20     ARM7 SHA1 HMAC hash 
    uint32_t mhmac_hash[5]; // 328h    20     Digest master SHA1 HMAC hash 
    uint32_t bhmac_hash[5]; // 33Ch    20     Banner SHA1 HMAC hash 
    uint32_t hmac_hash9d[5];// 350h    20     ARM9i (decrypted) SHA1 HMAC hash 
    uint32_t hmac_hash7d[5];// 364h    20     ARM7i (decrypted) SHA1 HMAC hash 
    uint8_t  reserve7[40];  // 378h    40     Reserved
    uint32_t hmac_hash9[5]; // 3A0h    20     ARM9 (without secure area) SHA1 HMAC hash 
                            // 3B4h    2636   Reserved 
                            // E00h    0x180  Reserved and unchecked region, always zero. Used for passing arguments in debug environment.
                            // F80h    0x80   RSA signature (first 0xE00 bytes of the header are signed with an 1024-bit RSA signature).
} nds_rom_hdr_t;

typedef struct __attribute__ ((packed)) {
    uint16_t version;       // 000h  2    Version  (0001h)
    uint16_t crc16;         // 002h  2    CRC16 across entries 020h..83Fh
    uint8_t  reserve1[0x1c];// 004h  1Ch  Reserved (zero-filled)
    uint8_t  icn_bmp[0x200];// 020h  200h Icon Bitmap  (32x32 pix) (4x4 tiles, each 4x8 bytes, 4bit depth)
    uint8_t  icn_pal[0x20]; // 220h  20h  Icon Palette (16 colors, 16bit, range 0000h-7FFFh)
                            //            (Color 0 is transparent, so the 1st palette entry is ignored)
    uint16_t title_jpn[0x80];// 240h  100h Title 0 Japanese (128 characters, 16bit Unicode)
    uint16_t title_eng[0x80];// 340h  100h Title 1 English  ("")
    uint16_t title_frn[0x80];// 440h  100h Title 2 French   ("")
    uint16_t title_grm[0x80];// 540h  100h Title 3 German   ("")
    uint16_t title_itl[0x80];// 640h  100h Title 4 Italian  ("")
    uint16_t title_spn[0x80];// 740h  100h Title 5 Spanish  ("")
    uint16_t title_chi[0x80];// 840h  100h Title 6 Chinese  ("") only if version = 2
//  uint8_t unknown[0x840]; //  940h  -    End of Icon/Title structure (next 1C0h bytes usually FFh-filled)
} nds_rom_icon_title_t;

typedef struct {
    nds_rom_hdr_t		hdr;
    nds_rom_icon_title_t	icon;
    unsigned int		filesystem_size;
    uint32_t                    file_crc32;
} nds_rom_info_t;


// PROTO
int main(int argc, char *argv[]);
int get_nds_info(FILE* rom_file, nds_rom_info_t *rom_info);
void print_rom_information(nds_rom_info_t *rom_info);
void get_title_info(uint16_t *title, char *str_buff);
void print_title_info(char *str_buff, const char *lang);
void get_unit_code(uint8_t unit_code, char *str_buff);
void get_maker_code(uint8_t *unit_code, char *str_buff);

int crc32fromFile(FILE *rom_file, uint32_t *outcrc32);
uint32_t crc32(uint32_t crc, const void *buf, size_t size);

void printv(const char *format,...);
void DBG(const char *format,...);
int parse_commandline(int argc, char *argv[]);
void help(void);

#endif // NDS_ROMINFO_H
