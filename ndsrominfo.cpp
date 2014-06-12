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

#include "ndsrominfo.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>


// options passed in via commandline
struct flag {
    unsigned int 	verbose:1;
    unsigned int 	debug:1;
    char 		*romfile;
};

#define OPTION_STR	"dhHvV:"
#define BUFFER_LEN	0x81
#define MB              (1024*1024)
#define WIFI_LEN        136         // Wifi data after end of rom, 136 bytes long

static flag opt_flags;

struct mctbl {
	uint8_t		C0;
	uint8_t		C1;
	const char *	Name;
};

struct Country {
	uint8_t		C;
	const char *	Name;
};

static uint32_t crc32_tab[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3,	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de,	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,	0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5,	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,	0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940,	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,	0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

static Country countries[] =
{
	{ 'J', "JPN" },
	{ 'E', "USA" },
	{ 'P', "EUR" },
	{ 'D', "NOE" },
	{ 'F', "NOE" },
	{ 'I', "ITA" },
	{ 'S', "SPA" },
	{ 'H', "HOL" },
	{ 'K', "KOR" },
	{ 'X', "EUU" },
};

static mctbl maker_table [] = {
	{ '0', '1', "Nintendo" },
	{ '0', '2', "Rocket Games, Ajinomoto" },
	{ '0', '3', "Imagineer-Zoom" },
	{ '0', '5', "Zamuse" },
	{ '0', '6', "Falcom" },
	{ '0', '8', "Capcom" },
	{ '0', '9', "Hot B Co." },
	{ '0', 'A', "Jaleco" },
	{ '0', 'B', "Coconuts Japan" },
	{ '0', 'C', "Coconuts Japan/G.X.Media" },
	{ '0', 'E', "Technos" },
	{ '0', 'F', "Mebio Software" },
	{ '0', 'G', "Shouei System" },
	{ '0', 'H', "Starfish" },
	{ '0', 'J', "Mitsui Fudosan/Dentsu" },
	{ '0', 'L', "Warashi Inc." },
	{ '0', 'N', "Nowpro" },
	{ '0', 'P', "Game Village" },
	{ '1', '2', "Infocom" },
	{ '1', '3', "Electronic Arts Japan" },
	{ '1', '5', "Cobra Team" },
	{ '1', '6', "Human/Field" },
	{ '1', '7', "KOEI" },
	{ '1', '8', "Hudson Soft" },
	{ '1', '9', "S.C.P." },
	{ '1', 'A', "Yanoman" },
	{ '1', 'C', "Tecmo Products" },
	{ '1', 'D', "Japan Glary Business" },
	{ '1', 'E', "Forum/OpenSystem" },
	{ '1', 'F', "Virgin Games" },
	{ '1', 'G', "SMDE" },
	{ '1', 'J', "Daikokudenki" },
	{ '1', 'P', "Creatures Inc." },
	{ '1', 'Q', "TDK Deep Impresion" },
	{ '2', '0', "Destination Software, KSS" },
	{ '2', '1', "Sunsoft/Tokai Engineering??" },
	{ '2', '2', "POW, VR 1 Japan??" },
	{ '2', '3', "Micro World" },
	{ '2', '5', "San-X" },
	{ '2', '6', "Enix" },
	{ '2', '7', "Loriciel/Electro Brain" },
	{ '2', '8', "Kemco Japan" },
	{ '2', '9', "Seta" },
	{ '2', 'A', "Culture Brain" },
	{ '2', 'C', "Palsoft" },
	{ '2', 'D', "Visit Co.,Ltd." },
	{ '2', 'E', "Intec" },
	{ '2', 'F', "System Sacom" },
	{ '2', 'G', "Poppo" },
	{ '2', 'H', "Ubisoft Japan" },
	{ '2', 'J', "Media Works" },
	{ '2', 'K', "NEC InterChannel" },
	{ '2', 'L', "Tam" },
	{ '2', 'M', "Jordan" },
	{ '2', 'Q', "Mediakite" },
	{ '3', '0', "Viacom" },
	{ '3', '1', "Carrozzeria" },
	{ '3', '2', "Dynamic" },
	{ '3', '4', "Magifact" },
	{ '3', '5', "Hect" },
	{ '3', '6', "Codemasters" },
	{ '3', '7', "Taito/GAGA Communications" },
	{ '3', '8', "Laguna" },
	{ '3', '9', "Telstar Fun & Games, Event/Taito" },
	{ '3', 'B', "Arcade Zone Ltd" },
	{ '3', 'C', "Entertainment International/Empire Software?" },
	{ '3', 'D', "Loriciel" },
	{ '3', 'E', "Gremlin Graphics" },
	{ '3', 'F', "K.Amusement Leasing Co." },
	{ '4', '0', "Seika Corp." },
	{ '4', '1', "Ubi Soft Entertainment" },
	{ '4', '4', "Life Fitness" },
	{ '4', '6', "System 3" },
	{ '4', '7', "Spectrum Holobyte" },
	{ '4', '9', "IREM" },
	{ '4', 'B', "Raya Systems" },
	{ '4', 'C', "Renovation Products" },
	{ '4', 'D', "Malibu Games" },
	{ '4', 'F', "Eidos (was U.S. Gold <=1995)" },
	{ '4', 'G', "Playmates Interactive?" },
	{ '4', 'J', "Fox Interactive" },
	{ '4', 'K', "Time Warner Interactive" },
	{ '4', 'Q', "Disney Interactive" },
	{ '4', 'S', "Black Pearl" },
	{ '4', 'U', "Advanced Productions" },
	{ '4', 'X', "GT Interactive" },
	{ '4', 'Z', "Crave Entertainment" },
	{ '5', '0', "Absolute Entertainment" },
	{ '5', '1', "Acclaim" },
	{ '5', '2', "Activision" },
	{ '5', '3', "American Sammy" },
	{ '5', '4', "Take 2 Interactive (before it was GameTek)" },
	{ '5', '5', "Hi Tech" },
	{ '5', '6', "LJN LTD." },
	{ '5', '8', "Mattel" },
	{ '5', 'B', "Romstar" },
	{ '5', 'C', "Taxan" },
	{ '5', 'D', "Midway (before it was Tradewest)" },
	{ '5', 'F', "American Softworks" },
	{ '5', 'G', "Majesco Sales Inc" },
	{ '5', 'H', "3DO" },
	{ '5', 'K', "Hasbro" },
	{ '5', 'L', "NewKidCo" },
	{ '5', 'M', "Telegames" },
	{ '5', 'N', "Metro3D" },
	{ '5', 'P', "Vatical Entertainment" },
	{ '5', 'Q', "LEGO Media" },
	{ '5', 'S', "Xicat Interactive" },
	{ '5', 'T', "Cryo Interactive" },
	{ '5', 'W', "Red Storm Entertainment" },
	{ '5', 'X', "Microids" },
	{ '5', 'Z', "Conspiracy/Swing" },
	{ '6', '0', "Titus" },
	{ '6', '1', "Virgin Interactive" },
	{ '6', '2', "Maxis" },
	{ '6', '4', "LucasArts Entertainment" },
	{ '6', '7', "Ocean" },
	{ '6', '9', "Electronic Arts" },
	{ '6', 'B', "Laser Beam" },
	{ '6', 'E', "Elite Systems" },
	{ '6', 'F', "Electro Brain" },
	{ '6', 'G', "The Learning Company" },
	{ '6', 'H', "BBC" },
	{ '6', 'J', "Software 2000" },
	{ '6', 'L', "BAM! Entertainment" },
	{ '6', 'M', "Studio 3" },
	{ '6', 'Q', "Classified Games" },
	{ '6', 'S', "TDK Mediactive" },
	{ '6', 'U', "DreamCatcher" },
	{ '6', 'V', "JoWood Produtions" },
	{ '6', 'W', "SEGA" },
	{ '6', 'X', "Wannado Edition" },
	{ '6', 'Y', "LSP" },
	{ '6', 'Z', "ITE Media" },
	{ '7', '0', "Infogrames" },
	{ '7', '1', "Interplay" },
	{ '7', '2', "JVC" },
	{ '7', '3', "Parker Brothers" },
	{ '7', '5', "Sales Curve" },
	{ '7', '8', "THQ" },
	{ '7', '9', "Accolade" },
	{ '7', 'A', "Triffix Entertainment" },
	{ '7', 'C', "Microprose Software" },
	{ '7', 'D', "Universal Interactive, Sierra, Simon & Schuster?" },
	{ '7', 'F', "Kemco" },
	{ '7', 'G', "Rage Software" },
	{ '7', 'H', "Encore" },
	{ '7', 'J', "Zoo Digital Publishing" },
	{ '7', 'K', "BVM" },
	{ '7', 'L', "Simon & Schuster Interactive" },
	{ '7', 'M', "Asmik Ace Entertainment Inc./AIA" },
	{ '7', 'Q', "Jester Interactive" },
	{ '7', 'T', "Scholastic" },
	{ '7', 'U', "Ignition Entertainment" },
	{ '7', 'W', "Stadlbauer" },
	{ '8', '0', "Misawa" },
	{ '8', '1', "Teichiku" },
	{ '8', '2', "Namco Ltd." },
	{ '8', '3', "LOZC" },
	{ '8', '4', "KOEI" },
	{ '8', '6', "Tokuma Shoten Intermedia" },
	{ '8', '7', "Tsukuda Original" },
	{ '8', '8', "DATAM-Polystar" },
	{ '8', 'B', "Bulletproof Software" },
	{ '8', 'C', "Vic Tokai Inc." },
	{ '8', 'E', "Character Soft" },
	{ '8', 'F', "I'Max" },
	{ '8', 'G', "Saurus" },
	{ '8', 'J', "General Entertainment" },
	{ '8', 'N', "Success" },
	{ '8', 'P', "SEGA Japan" },
	{ '9', '0', "Takara Amusement" },
	{ '9', '1', "Chun Soft" },
	{ '9', '3', "BEC" },
	{ '9', '5', "Varie" },
	{ '9', '6', "Yonezawa/S'pal" },
	{ '9', '7', "Kaneko" },
	{ '9', '9', "Victor Interactive Software, Pack in Video" },
	{ '9', 'A', "Nichibutsu/Nihon Bussan" },
	{ '9', 'B', "Tecmo" },
	{ '9', 'C', "Imagineer" },
	{ '9', 'F', "Nova" },
	{ '9', 'G', "Den'Z" },
	{ '9', 'H', "Bottom Up" },
	{ '9', 'J', "TGL" },
	{ '9', 'N', "Marvelous Entertainment" },
	{ '9', 'P', "Keynet Inc." },
	{ '9', 'Q', "Hands-On Entertainment" },
	{ 'A', '0', "Telenet" },
	{ 'A', '1', "Hori" },
	{ 'A', '4', "Konami" },
	{ 'A', '5', "K.Amusement Leasing Co." },
	{ 'A', '6', "Kawada" },
	{ 'A', '7', "Takara" },
	{ 'A', '9', "Technos Japan Corp." },
	{ 'A', 'A', "JVC, Victor Musical Indutries" },
	{ 'A', 'C', "Toei Animation" },
	{ 'A', 'D', "Toho" },
	{ 'A', 'F', "Namco" },
	{ 'A', 'G', "Media Rings Corporation" },
	{ 'A', 'H', "J-Wing" },
	{ 'A', 'J', "Pioneer LDC" },
	{ 'A', 'K', "KID" },
	{ 'A', 'L', "Mediafactory" },
	{ 'A', 'P', "Infogrames Hudson" },
	{ 'A', 'Q', "Kiratto. Ludic Inc" },
	{ 'B', '0', "Acclaim Japan" },
	{ 'B', '1', "ASCII" },
	{ 'B', '2', "Bandai" },
	{ 'B', '4', "Enix" },
	{ 'B', '6', "HAL Laboratory" },
	{ 'B', '7', "SNK" },
	{ 'B', '9', "Pony Canyon" },
	{ 'B', 'A', "Culture Brain" },
	{ 'B', 'B', "Sunsoft" },
	{ 'B', 'C', "Toshiba EMI" },
	{ 'B', 'D', "Sony Imagesoft" },
	{ 'B', 'F', "Sammy" },
	{ 'B', 'G', "Magical" },
	{ 'B', 'H', "Visco" },
	{ 'B', 'J', "Compile " },
	{ 'B', 'L', "MTO Inc." },
	{ 'B', 'N', "Sunrise Interactive" },
	{ 'B', 'P', "Global A Entertainment" },
	{ 'B', 'Q', "Fuuki" },
	{ 'C', '0', "Taito" },
	{ 'C', '2', "Kemco" },
	{ 'C', '3', "Square" },
	{ 'C', '4', "Tokuma Shoten" },
	{ 'C', '5', "Data East" },
	{ 'C', '6', "Tonkin House (was Tokyo Shoseki)" },
	{ 'C', '8', "Koei" },
	{ 'C', 'A', "Konami/Ultra/Palcom" },
	{ 'C', 'B', "NTVIC/VAP" },
	{ 'C', 'C', "Use Co.,Ltd." },
	{ 'C', 'D', "Meldac" },
	{ 'C', 'E', "Pony Canyon" },
	{ 'C', 'F', "Angel, Sotsu Agency/Sunrise" },
	{ 'C', 'J', "Boss" },
	{ 'C', 'G', "Yumedia/Aroma Co., Ltd" },
	{ 'C', 'K', "Axela/Crea-Tech?" },
	{ 'C', 'M', "Konami Computer Entertainment Osaka" },
	{ 'C', 'P', "Enterbrain" },
	{ 'D', '0', "Taito/Disco" },
	{ 'D', '1', "Sofel" },
	{ 'D', '2', "Quest, Bothtec" },
	{ 'D', '4', "Ask Kodansha" },
	{ 'D', '6', "Naxat" },
	{ 'D', '7', "Copya System" },
	{ 'D', '8', "Capcom Co., Ltd." },
	{ 'D', '9', "Banpresto" },
	{ 'D', 'A', "TOMY" },
	{ 'D', 'B', "LJN Japan" },
	{ 'D', 'D', "NCS" },
	{ 'D', 'E', "Human Entertainment" },
	{ 'D', 'F', "Altron" },
	{ 'D', 'H', "Gaps Inc." },
	{ 'D', 'N', "Elf" },
	{ 'E', '0', "Jaleco" },
	{ 'E', '2', "Yutaka" },
	{ 'E', '3', "Varie" },
	{ 'E', '4', "T&ESoft" },
	{ 'E', '5', "Epoch" },
	{ 'E', '7', "Athena" },
	{ 'E', '8', "Asmik" },
	{ 'E', '9', "Natsume" },
	{ 'E', 'A', "King Records" },
	{ 'E', 'B', "Atlus" },
	{ 'E', 'C', "Epic/Sony Records" },
	{ 'E', 'E', "IGS" },
	{ 'E', 'G', "Chatnoir" },
	{ 'E', 'H', "Right Stuff" },
	{ 'E', 'L', "Spike" },
	{ 'E', 'M', "Konami Computer Entertainment Tokyo" },
	{ 'E', 'N', "Alphadream Corporation" },
	{ 'F', '0', "A Wave" },
	{ 'F', '1', "Motown Software" },
	{ 'F', '2', "Left Field Entertainment" },
	{ 'F', '3', "Extreme Ent. Grp." },
	{ 'F', '4', "TecMagik" },
	{ 'F', '9', "Cybersoft" },
	{ 'F', 'B', "Psygnosis" },
	{ 'F', 'E', "Davidson/Western Tech." },
	{ 'G', '1', "PCCW Japan" },
	{ 'G', '4', "KiKi Co Ltd" },
	{ 'G', '6', "Sims" },
	{ 'G', '7', "Broccoli" },
	{ 'G', '8', "Avex" },
	{ 'G', '9', "D3 Publisher" },
	{ 'G', 'B', "Konami Computer Entertainment Japan" },
	{ 'G', 'D', "Square-Enix" },
	{ 'I', 'H', "Yojigen" },
//	{'', '', ""},
	{0, 0, NULL}
};

/***************************************************************************
 * Main
 ***************************************************************************/
int main(int argc, char *argv[])
{
	int		res;
	nds_rom_info_t	rom_info;
	FILE *		rom_file;


	// Clean Structs, ect ...
	res = 0;
	memset(&opt_flags, 0, sizeof(struct flag));
	memset(&rom_info,0,sizeof(nds_rom_info_t));
	rom_file = NULL;

	// Resolve Args
	if (parse_commandline(argc, argv) == 1)
	{
		return (0);
	}

	// Open nds file
	DBG("Open nds file:%s\n", opt_flags.romfile);
	if ((rom_file=fopen(opt_flags.romfile, "rb")) == NULL)
        {
		perror(opt_flags.romfile);
		res = -1;
		goto main_cleanup;
	}

	// populate ROM info structure
	if (get_nds_info(rom_file, &rom_info) !=0 )
	{
        	std::cout << "Bad ROM header. Could not read ROM information." << std::endl;
		res = -1;
		goto main_cleanup;
	}

	// Now calculate crc32
	fseek(rom_file, 0, SEEK_SET);
	if (crc32fromFile(rom_file, &rom_info.file_crc32) != 0)
	{
		rom_info.file_crc32 = 0;
	}

	// Now Print Rom Info
	print_rom_information(&rom_info);

	// Cleanup
main_cleanup:

	
	if (rom_file != NULL )
	{
		DBG("Closing nds file.\n");
		if (fclose(rom_file) == EOF)
		{
			perror(opt_flags.romfile);
		}
	}

	if (opt_flags.romfile != NULL)
	{
		DBG("Cleanup allocated memory.\n");
		delete []opt_flags.romfile;
		opt_flags.romfile = NULL;
	}

	return (res);
}

/***************************************************************************
 * get_nds_info
 ***************************************************************************/
int get_nds_info(FILE* rom_file, nds_rom_info_t *rom_info)
{
	int		res;
	unsigned int	total_bytes_read;
	uint32_t	icon_title_offset;

	DBG("%s called'.\n",__FUNCTION__);

	res = 0;
	total_bytes_read = 0;
	icon_title_offset = 0;

	do
	{
		DBG("Check FILE and rom_info struct.\n");
		if ((rom_info == NULL) || (rom_file == NULL))
        	{
			res = -1;
			break;
		}

		//
        	// read in header, should only read in one item
        	//
		DBG("Read rom header from file.\n");
		total_bytes_read = fread(&rom_info->hdr, 1, sizeof(nds_rom_hdr_t), rom_file);
		DBG("Bytes read:%d.\n", total_bytes_read);
        	if (total_bytes_read != sizeof(nds_rom_hdr_t))
        	{
			res = -1;
			break;
		}

		DBG("Get icon title offset.\n");
		icon_title_offset = endian_32(rom_info->hdr.icon_title_off);
		if (icon_title_offset != 0 )
		{
			DBG("Icon title offset at:%d\n", icon_title_offset);
			if (fseek(rom_file, icon_title_offset, SEEK_SET))
			{
				res = -1;
				break;
			}
			DBG("Read rom Icon title.\n");
			if ((fread(&rom_info->icon, sizeof(nds_rom_icon_title_t), 1, rom_file)) != 1)
			{
				res = -1;
				break;
			}
		}
		else
		{
			printv("No icon title offset.\n");
		}

		DBG("Get rom file size.\n");
		if (fseek(rom_file, 0, SEEK_END) == 0)
		{
			rom_info->filesystem_size = ftell(rom_file);
		}
		else
		{
			perror("fseek");
		}

		break;
	}
	while (0);

	return (res);
}

/***************************************************************************
 * print_rom_information
 ***************************************************************************/
void print_rom_information(nds_rom_info_t *rom_info)
{
	char		str_buff[BUFFER_LEN];
	int		i;
	unsigned int	rom_size;
	unsigned int	total_rom_size;
	int 		NumCountries;

	if (rom_info == NULL)
	{
		std::cout << "Internal error." << std::endl;
		return;
	}

	i = 0;
	rom_size = 0;
	total_rom_size = 0;
	NumCountries = sizeof(countries) / sizeof(countries[0]);

	memset(str_buff, 0, BUFFER_LEN);
	memcpy(str_buff, (char *)rom_info->hdr.game_title, sizeof(rom_info->hdr.game_title));
	std::cout << "Internal ROM name        : " << str_buff << std::endl;

	// Game Title in native Languages
	memset(str_buff, 0, BUFFER_LEN);
	get_title_info(rom_info->icon.title_jpn, str_buff);
	print_title_info(str_buff, "JPN");

	memset(str_buff, 0, BUFFER_LEN);
	get_title_info(rom_info->icon.title_eng, str_buff);
	print_title_info(str_buff, "ENG");

	memset(str_buff, 0, BUFFER_LEN);
	get_title_info(rom_info->icon.title_frn, str_buff);
	print_title_info(str_buff, "FRN");

	memset(str_buff, 0, BUFFER_LEN);
	get_title_info(rom_info->icon.title_grm, str_buff);
	print_title_info(str_buff, "GER");

	memset(str_buff, 0, BUFFER_LEN);
	get_title_info(rom_info->icon.title_itl, str_buff);
	print_title_info(str_buff, "ITL");

	memset(str_buff, 0, BUFFER_LEN);
	get_title_info(rom_info->icon.title_spn, str_buff);
	print_title_info(str_buff, "SPN");

	// Have V2 Rom Type?
	if (rom_info->icon.version > 1)
	{
		memset(str_buff, 0, BUFFER_LEN);
		get_title_info(rom_info->icon.title_chi, str_buff);
		print_title_info(str_buff, "CHI");	
	}

	// Game Code
	memset(str_buff, 0, BUFFER_LEN);
	memcpy(str_buff, (char *)rom_info->hdr.game_code, sizeof(rom_info->hdr.game_code));
	strcat(str_buff, "-");
	for (i=0; i<NumCountries; i++)
	{
		if (countries[i].C == rom_info->hdr.game_code[3])
		{
			strcat(str_buff, countries[i].Name);
			break;
		}
	}
	std::cout << "Game code                : NTR-" << str_buff << std::endl;

	// Maker Code
	memset(str_buff, 0, BUFFER_LEN);
	memcpy(str_buff, (char *)rom_info->hdr.maker_code, sizeof(rom_info->hdr.maker_code));
	std::cout << "Maker code               : " << str_buff << std::endl;
	memset(str_buff, 0, BUFFER_LEN);
	get_maker_code(rom_info->hdr.maker_code, str_buff);
	std::cout << "                         : " << str_buff << std::endl;

	// Filesize
	std::cout << "Filesystem ROM size      : " << rom_info->filesystem_size << " Bytes" << std::endl;
	std::cout << "                         : " << rom_info->filesystem_size/(float)MB << " MB" << std::endl;

	rom_size = endian_32(rom_info->hdr.rom_size);
	total_rom_size = rom_size + WIFI_LEN;

	if (opt_flags.verbose)
	{
		// Rom sizes
		std::cout << "Internal ROM size        : " << rom_size << " Bytes" << std::endl;
		std::cout << "                         : " << rom_size/(float)MB << " MB" << std::endl;
	
		std::cout << "Internal ROM size + wifi : " << total_rom_size << " Bytes" << std::endl;
		std::cout << "                         : " << total_rom_size/(float)MB << " MB" << std::endl;
	}

	// Unitcode
	memset(str_buff, 0, BUFFER_LEN);
	get_unit_code(rom_info->hdr.unit_code, str_buff);
	std::cout << "Unitcode                 : " << str_buff << std::endl;

	// CRC32
	memset(str_buff, 0, BUFFER_LEN);
	sprintf(str_buff, "%X", rom_info->file_crc32);
	std::cout << "CRC32                    : " << str_buff << std::endl;

	// Capacity
	memset(str_buff, 0, BUFFER_LEN);
	sprintf(str_buff, "%.0f", (((128*1024) << rom_info->hdr.dev_capacity)*8/(float)MB));
	std::cout << "Capacity                 : " << str_buff << " MBit" << std::endl;

}

/***************************************************************************
 * get_title_info
 ***************************************************************************/
void get_title_info(uint16_t *title, char *str_buff)
{
	int	i;

	for (i=0; i<128; i++)
	{
		if (title[i] == 0)
		{
			break;
		}
		str_buff[i] = endian_16(title[i]);
	}
}

/***************************************************************************
 * print_title_info
 ***************************************************************************/
void print_title_info(char *str_buff, const char *lang)
{
	int	i;
	char *	str_buff_ptr;
	char *	tmpstr;

	i = 0;
	str_buff_ptr = NULL;
	tmpstr = NULL;

	str_buff_ptr = str_buff;

	while ((tmpstr = strtok(str_buff_ptr, "\n")) != NULL)
	{
		if (i++ == 0)
		{
			std::cout << "Internal title [" << lang << "]     : " << tmpstr << std::endl;
			str_buff_ptr = NULL;
		}
		else
		{
			std::cout << "                         : " << tmpstr << std::endl;
		}
	}
}

/***************************************************************************
 * get_unit_code
 ***************************************************************************/
void get_unit_code(uint8_t unit_code, char *str_buff)
{
	switch(unit_code)
	{
		case 0:
		{
			strcpy(str_buff, "Nintendo DS");
			break;
		}
		default:
		{
			// Unknown show Number
			sprintf(str_buff, "Unknown [%X]", unit_code);
			break;
		}
	}
}

/***************************************************************************
 * get_maker_code
 ***************************************************************************/
void get_maker_code(uint8_t *unit_code, char *str_buff)
{
	int	i;

	i = 0;

	do
	{
		if (maker_table[i].C0 == unit_code[0])
		{
			if (maker_table[i].C1 == unit_code[1])
			{
				strcpy(str_buff, maker_table[i].Name);
				break;
			}
		}
		i++;
	}
	while (maker_table[i].Name != NULL);

	if (maker_table[i].Name == NULL)
	{
		strcpy(str_buff, "Unknown");
	}
}

/***************************************************************************
 * 
 ***************************************************************************/

/***************************************************************************
 * 
 ***************************************************************************/

/***************************************************************************
 * crc32fromFile
 ***************************************************************************/
int crc32fromFile(FILE *rom_file, uint32_t *outcrc32 )
{

#define CRC_BUFFER_SIZE  8192

    unsigned char buf[CRC_BUFFER_SIZE];
    size_t bufLen;

    /** accumulate crc32 from file **/
    *outcrc32 = 0;
    while (1) {
        bufLen = fread( buf, 1, CRC_BUFFER_SIZE, rom_file );
        if (bufLen == 0) {
            if (ferror(rom_file)) {
                fprintf( stderr, "error reading file\n" );
                goto ERR_EXIT;
            }
            break;
        }
        *outcrc32 = crc32( *outcrc32, buf, bufLen );
    }
    return( 0 );

    /** error exit **/
ERR_EXIT:
    return( -1 );
}

/***************************************************************************
 * crc32
 ***************************************************************************/
uint32_t crc32(uint32_t crc, const void *buf, size_t size)
{
	const uint8_t *p;

	p = (const uint8_t *)buf;
	crc = crc ^ ~0U;

	while (size--)
		crc = crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);

	return crc ^ ~0U;
}

/***************************************************************************
 * printv
 ***************************************************************************/
void printv(const char *format,...)
{
    va_list ap;
    if (opt_flags.verbose == 1)
    {
        va_start(ap,format);
        vprintf(format,ap);
        va_end(ap);
    }
}

/***************************************************************************
 * DBG
 ***************************************************************************/
void DBG(const char *format,...)
{
    va_list ap;
    if (opt_flags.debug == 1)
    {
        va_start(ap,format);
        vprintf(format,ap);
        va_end(ap);
    }
}

/***************************************************************************
 * parse_commandline
 ***************************************************************************/
int parse_commandline(int argc, char *argv[])
{
	int opt;

	while ((opt = getopt(argc, argv, OPTION_STR)) != -1)
	{
		switch (opt)
		{
			case 'd':
			{
				opt_flags.debug = 1;
				break;
			}
			case 'h':
			case 'H':
			{
				help();
				return (1);
			}
			case 'v':
			case 'V':
			{
				opt_flags.verbose = 1;
				break;
			}
		}
	}

	// Check for nds file
	if (optind >= argc) 
	{
        	std::cout << "Missing nds file!" << std::endl;
        	return (1);
	}

	// Get ndsfile
	DBG("Allocate Memory.\n");
	opt_flags.romfile = new char[strlen(argv[optind])+1];
	memset(opt_flags.romfile, 0, strlen(argv[optind])+1);
	memcpy(opt_flags.romfile, argv[optind], strlen(argv[optind]));
	
	DBG("Flags: verbose=%d, debug=%d\n",
		opt_flags.verbose,opt_flags.debug);

	DBG("NDS file: %s\n",
		opt_flags.romfile);

	return (0);
}

/***************************************************************************
 * help
 ***************************************************************************/
void help(void)
{
	std::cout << "NDSrominfo -hHvV <romfile>" << std::endl;
	std::cout << "--------------------------" << std::endl;
	std::cout << "-h Help" << std::endl;
	std::cout << "-H Help" << std::endl;
	std::cout << "-v Verbose" << std::endl;
	std::cout << "-V Verbose" << std::endl;
	std::cout << std::endl;
	std::cout << "(C) 2013 by IT-Solutions" << std::endl;
	std::cout << "(W) 2013 by Christoph Gembalski" << std::endl;
	std::cout << "            christoph.gembalski@web.de" << std::endl;
}

