#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
// colors
#define BLACK 30
#define RED 31
#define GREEN 32
#define YELLOW 33
#define BLUE 34
#define MAGENTA 35
#define CYAN 36
#define WHITE 37
#define DEFAULT_COL BLACK
// tile nums
#define T_AIR 0
#define T_WALL 1
#define T_BORDER 2 // solid wall at the edge of map
#define T_LAVA 3
// item nums
#define I_NOTHING 0
// generic (red potion, yew wand)
#define I_POTION_RED 1
#define I_POTION_GREEN 2
#define I_DEBUG_GE 3
// specific (healing potion, lightning wand)
#define I_POTION_DEATH 4
#define I_POTION_NOTHING 5
#define I_DEBUG_SP 6
// neither (armor and weapons)
#define I_ARMOR_1 7
#define I_ARMOR_2 8
#define I_ARMOR_3 9
#define I_WEAPON_1 10
#define I_WEAPON_2 11
#define I_WEAPON_3 12
int ARMOR_SS[] = {0,10,11,12};
int WEAPON_SS[] = {0,10,11,12};
#define NUM_OF_IDEN_ITEMS 3
#define NUM_CATS 2
// 447135
#define MONSTER_BASIC 1
int MONST_STRS[] = {0,10};
int MONST_MAX[] = {0,20};
#define D_WIDTH 20
#define D_HEIGHT 20

int FIRST_GENERICS[] = {I_POTION_RED,I_DEBUG_GE};

char *times(char *str, int reps) { // repeats a string a certain number of times.
	char *buf = (char *)malloc(1024);
	*buf = 0;
	for (int i = 0; i < reps; i++) {
		buf = strcat(buf,str);
	}
	return buf;
}
void printc(char *str, int color) { // color printing, uses DEFINE macros for colors
	printf("\x1b\x5b%dm",color);
	printf("%s",str);
	printf("\x1b\x5b%dm",DEFAULT_COL);
}
void color_set(int color) { // enables a color
	printf("\x1b\x5b%dm",color);
}
void color_reset() { // back to default
	printf("\x1b\x5b%dm",DEFAULT_COL);
}
// some ANSI escape sequences
char *ANSI_up = "\x1b\x5b""A";
char *ANSI_down = "\x1b\x5b""B";
char *ANSI_right = "\x1b\x5b""C";
char *ANSI_left = "\x1b\x5b""D";
char *ANSI_next = "\x1b\x5b""E";
char *ANSI_prev = "\x1b\x5b""F";
char *ANSI_clr = "\x1b\x5b""2J";
char *ANSI_clr_ln = "\x1b\x5b""2K";
char *ANSI_scr_up = "\x1b\x5b""S";
char *ANSI_scr_down = "\x1b\x5b""T";
char *ANSI_home = "\x1b\x5b""H";
// tileset
char TILES[] = " ##:";
int T_COLORS[] = {WHITE,WHITE,BLUE,RED};
#define SOLID 1
int T_FLAGS[] = {0,SOLID,SOLID,0}; 
// itemset and items on map
int items[D_WIDTH][D_HEIGHT];
char *ITEM_CHARS = " !!*!!*@@@";
int ITEM_COLS[] = {WHITE,RED,GREEN,BLUE,RED,GREEN,BLUE,WHITE,WHITE,WHITE};
char *ITEM_NAMES[] = {"nothing","red potion","green potion","DEBUG","potion of death","potion of nothing","DEBUG2","armor 1","armor 2","armor 3"};
// dungeon
int dungeon[D_WIDTH][D_HEIGHT];

// just some bools to make life easier
typedef int bool;
#define true 1;
#define false 0;
bool tile_flag(int tile_x, int tile_y, int test_flag) { // retrieve tile info
	return !!(T_FLAGS[dungeon[tile_y][tile_x]] & test_flag);
}

// monsters
int monsters[D_WIDTH][D_HEIGHT];
int mdamages[D_WIDTH][D_HEIGHT];
char *MONST_CHARS = " B";
int MONST_COLS[] = {WHITE,RED};
char *MONST_NAMES[] = {"","random thing"};

// player
int player_x;
int player_y;
void enforce_borders() { // set the edge tiles to T_BORDER
	for (int y = 0; y < D_HEIGHT; y++) {
		if (y == 0 || y == D_HEIGHT-1) { // top or bottom need all tiles T_BORDER
			for (int x = 0; x < D_WIDTH; x++) {
				dungeon[y][x] = T_BORDER;
			}
		} else {
			dungeon[y][0] = T_BORDER;
			dungeon[y][D_HEIGHT-1] = T_BORDER;
		}
	}
}
int fill_tiles(int left_x, int top_y, int width, int height, int fill_with) { // a rectangle will be filled
	if (left_x < 0 || top_y < 0 || width < 0 || height < 0 || left_x + width > D_WIDTH || top_y + height > D_HEIGHT) {
		return 1; // invalid position
	} else { // valid position
		for (int x = left_x; x < left_x + width; x++) {
			for (int y = top_y; y < top_y + height; y++) {
				dungeon[y][x] = fill_with; // the tile to fill with
			}
		}
		return 0;
	}
}
void display_dungeon(int left_x, int top_y) { 
	// DISPLAYS THE DUNGEON (wow)
	printf("%s%s%s%s", ANSI_home, ANSI_down, times(ANSI_right, left_x), times(ANSI_down, top_y));
	// now we're at the starting position
	for (int i = 0; i < D_HEIGHT; i++) {
		for (int j = 0; j < D_WIDTH; j++) {
			color_set(T_COLORS[dungeon[i][j]]);
			printf("%c",TILES[dungeon[i][j]]);
			color_reset();
		}
		printf("\n");
	}
}
void display_player() {
	printf("%s%s", ANSI_home, ANSI_down); // down to skip HP bar
	printf("%s%s", times(ANSI_right, player_x), times(ANSI_down, player_y));
	printf("@");
	printf("%s",times(ANSI_next,D_HEIGHT-player_y));
}
// retrieved from site: https://stackoverflow.com/questions/7469139/what-is-equivalent-to-getch-getche-in-linux
// not written by us
#include <termios.h>

static struct termios old, new;
/* Initialize new terminal i/o settings */
void initTermios(int echo) 
{
  tcgetattr(0, &old); /* grab old terminal i/o settings */
  new = old; /* make new settings same as old settings */
  new.c_lflag &= ~ICANON; /* disable buffered i/o */
  if (echo) {
      new.c_lflag |= ECHO; /* set echo mode */
  } else {
      new.c_lflag &= ~ECHO; /* set no echo mode */
  }
  tcsetattr(0, TCSANOW, &new); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void) 
{
  tcsetattr(0, TCSANOW, &old);
}

/* Read 1 character - echo defines echo mode */
char getch_(int echo) 
{
  char ch;
  initTermios(echo);
  ch = getchar();
  resetTermios();
  return ch;
}

// back to our stuff

// info messages
char *infos[5] = {"-","-","-","-","-"};
void pr_info(char *msg) {
	char *s1 = (char *)malloc(128); // temp strings
	char *s2 = (char *)malloc(128); // use malloc so they don't go out of scope
	char *s3 = (char *)malloc(128);
	char *s4 = (char *)malloc(128);
	char *s5 = (char *)malloc(128);
	strcpy(s1,infos[1]); // shift one place back
	strcpy(s2,infos[2]);
	strcpy(s3,infos[3]);
	strcpy(s4,infos[4]);
	strcpy(s5,msg); // message is last place
	infos[0] = s1; // put back in infos[]
	infos[1] = s2;
	infos[2] = s3;
	infos[3] = s4;
	infos[4] = s5;
}
int death = 0;
bool important_msg = false;
char *death_reason;
int iden_generics[NUM_OF_IDEN_ITEMS];
int iden_specifics[NUM_OF_IDEN_ITEMS];
bool near(int ax, int ay, int bx, int by) {
	return (ax == bx && abs(ay-by)==1) || (ay == by && abs(ax-bx)==1);
}
bool is_contained_in(int arr[], int size, int val) { // is the val contained in the arr?
	for (int idx = 0; idx < size; idx++) {
		if (arr[idx] == val) {
			return idx+1;
		}
	}
	return 0;
}
void shuffle_rand_items() { // generates identifiable items
	for(int i = 0; i < NUM_OF_IDEN_ITEMS; i++) {
		int cat_num = 0;
		while (i+FIRST_GENERICS[0] >= FIRST_GENERICS[cat_num]) cat_num++; // find which category generic is in
		cat_num--;
		printf("i=%d CATEGORY: %d\n",i,cat_num);
		int min_val = FIRST_GENERICS[cat_num]; // first item of category
		int max_val = cat_num+1 == NUM_CATS ? NUM_OF_IDEN_ITEMS : FIRST_GENERICS[cat_num+1]-1; // last item of category
		printf("from %d-%d\n",min_val,max_val);
		int rand_val;
		do {
			rand_val = rand()%(max_val-min_val+1)+min_val;
			printf("poop %d\n",rand_val);
		} while (is_contained_in(iden_specifics, NUM_OF_IDEN_ITEMS, rand_val+NUM_OF_IDEN_ITEMS)); // while value is already used
		iden_generics[i] = i+FIRST_GENERICS[0]; // whichever item operating on
		iden_specifics[i] = rand_val+NUM_OF_IDEN_ITEMS; // corresponding specific
	}
}
void display_generic_specific() { // solely debug
	for (int i = 0; i < NUM_OF_IDEN_ITEMS; i++) {
		color_set(MAGENTA);
		printf("GENERIC: %d, SPECIFIC: %d\n",iden_generics[i],iden_specifics[i]);
		color_reset();
	}
}
void display_items(int itemlist[D_HEIGHT][D_WIDTH]) {
	for (int y = 0; y < D_HEIGHT; y++) {
		for (int x = 0; x < D_WIDTH; x++) {
			if (itemlist[y][x]) { // an item exists there
				char ic = ITEM_CHARS[itemlist[y][x]];
				printf("%s%s", ANSI_home, ANSI_down); // skip HP bar
				printf("%s%s", times(ANSI_right, x), times(ANSI_down, y));
				color_set(ITEM_COLS[itemlist[y][x]]);
				printf("%c",ic);
				color_reset();
				printf("%s",times(ANSI_next,D_HEIGHT-player_y));
			}
		}
	}
}
void display_monsts(int monstlist[D_HEIGHT][D_WIDTH]) { // copy of items pretty much
	for (int y = 0; y < D_HEIGHT; y++) {
		for (int x = 0; x < D_WIDTH; x++) {
			if (monstlist[y][x]) { // a monster exists there
				// scan for dead monsters also
				if (mdamages[y][x] >= MONST_MAX[monstlist[y][x]]) {
					char buf[256];
					sprintf(buf,"The %s dies!",MONST_NAMES[monstlist[y][x]]);
					pr_info(buf);
					monstlist[y][x] = 0;
				}
				char mc = MONST_CHARS[monstlist[y][x]];
				printf("%s%s", ANSI_home, ANSI_down); // skip HP bar
				printf("%s%s", times(ANSI_right, x), times(ANSI_down, y));
				color_set(MONST_COLS[monstlist[y][x]]);
				printf("%c",mc);
				color_reset();
				printf("%s",times(ANSI_next,D_HEIGHT-player_y));
			}
		}
	}
}
int armor_on = 0;
int weapon_on = 0;
int playerStr = 10;
int attack(int strengthAtk, int strengthDef, char *monstName, bool you) {
	if (you) {
		int sval = WEAPON_SS[weapon_on]-playerStr;
		strengthAtk += weapon_on - sval>0?sval:0;
	} else {
		int sval = ARMOR_SS[armor_on]-playerStr;
		strengthDef += armor_on - sval>0?sval:0;
	}

	strengthAtk += 6;
	int missChance = (strengthAtk-strengthDef)/2;
	int val = rand()%missChance;
	char theName[100];
	sprintf(theName,"The %s",monstName);
	if (val) { // hit
		sprintf(theName,"%s hit%s!",you?"You":theName,you?"":"s");
		pr_info(theName);
		return val*strengthAtk/(strengthDef/4);
	} else { // miss
		sprintf(theName,"%s miss%s!",you?"You":theName,you?"":"es");
		pr_info(theName);
		return 0;
	}
}
// useful function, this is
// for example:
// str_format("hello",0,1) => "hello"
// str_format("hello",1,1) => "a hello"
// str_format("ohio",1,1) => "an ohio"
// str_format("hello",2,1) => "the hello"
// str_format("hello",0,10) => "10 hellos"
// str_format("hello",1,10) => "10 hellos"
// str_format("hello",2,10) => "the 10 hellos"
// str_format("hello world",0,10) => "10 hello worlds"
// str_format("hello of world",0,10) => "10 hellos of world"
// str_format("foo bar of baz",0,10) => "10 foo bars of baz"
char *str_format(char *str, int article, int quantity) {
	char *buffer = (char *)malloc(256); // malloc so it doesn't go out of scope
	if (quantity == 1) {
		// singular
		// the ternary expressions are really long and hard to understand
		sprintf(buffer,"%s %s",(article?(article==2?"the":((str[0] == 'a' || str[0] == 'e' || str[0] == 'i' || str[0] == 'o' || str[0] == 'u')?"an":"a")):""),str);
	} else {
		// pluralize
		char *plural = (char *)malloc(256);
		char *of_sep = strstr(str," of ");
		strcpy(plural,"");
    if (!of_sep) { // no "of", null ptr
      strcat(plural,str);
      strcat(plural,"s");
    } else {
		  *of_sep = '\0'; // cut off
		  strcat(plural,str); // put everything before the of
		  strcat(plural,"s of "); // pluralize it
		  *of_sep = ' '; // don't overall change str
		  of_sep += 4; // rest of str
		  strcat(plural,of_sep); // append rest of string
    }
		if (quantity == 0) {
			sprintf(buffer,"%sno %s",article==2?"the ":"",plural);
		} else {
			sprintf(buffer,"%s%d %s",article==2?"the ":"",quantity,plural);
		}
	}
	return buffer;
}
//inventory
int pack_items[20];
int pack_counts[20];
int num_pack_slots = 0;
void add_item(int item, int quantity) { // add an item to your pack
	char buffer[255];
	if (is_contained_in(pack_items,20,item)) { // already exists
    int prev_amt = pack_counts[is_contained_in(pack_items,20,item)-1];
		pack_counts[is_contained_in(pack_items,20,item)-1] += quantity; // add some more to the stack
		sprintf(buffer,"You now have %s.",str_format(ITEM_NAMES[item],0,prev_amt+quantity));
		pr_info(buffer);
	} else { // new item
		if (num_pack_slots == 20) {
			pr_info("You don't have enough space in your pack.");
		} else {
			pack_items[num_pack_slots] = item;
			pack_counts[num_pack_slots] = quantity;
			num_pack_slots++;
			sprintf(buffer,"Item #%d: %s.",num_pack_slots,str_format(ITEM_NAMES[item],1,quantity));
			pr_info(buffer);
		}
	}
}
void del_item(int item, int quantity) { // delete items from the pack
	// THIS FUNCTION DOESN'T WORK, WILL BE FIXED
	// i think it works now (?)
	char buffer[255];
	if (!is_contained_in(pack_items,20,item)) {
		pr_info("You don't have that.");
	} else {
		if (pack_counts[is_contained_in(pack_items,20,item)-1] < quantity) { // not enough to remove that many
			pr_info("You don't have enough of that to remove that many!");
		} else {
			sprintf(buffer,"You now have %s.",str_format(ITEM_NAMES[item],1,pack_counts[is_contained_in(pack_items,20,item)-1]-quantity));
			pr_info(buffer);
			if (pack_counts[is_contained_in(pack_items,20,item)-1] == quantity) {
				int idx_of = is_contained_in(pack_items,20,item);
				for (int i = idx_of; i < num_pack_slots; i++) {
					pack_items[i-1] = pack_items[i];
					pack_counts[i-1] = pack_counts[i];
				}
				num_pack_slots--;
			} else {
				pack_counts[is_contained_in(pack_items,20,item)-1] -= quantity; // take some from the stack
			}
		}
	}
}
void take_inventory(int empty_lines) {
	printf("%s%sInventory: %d items\n",times(ANSI_down, empty_lines), ANSI_home,num_pack_slots);
	for (int i = 0; i < num_pack_slots; i++) {
		printf("Item %d: %s\n",i+1, str_format(ITEM_NAMES[pack_items[i]],1,pack_counts[i]));
	}
}
int hp = 100; // hit points
int maxhp = 100;

// monster ais
void basic_monst_ai(int x, int y) {
	if (!tile_flag(x-1,y,SOLID)) {
	monsters[y][x-1] = MONSTER_BASIC;
	monsters[y][x] = 0;
	}
	if (near(x, y, player_x, player_y)) {
		hp -= attack(10, playerStr, "random thing", 0);
		death_reason = "a random thing";
	}
}
void (*MONST_AIS[])(int x, int y) = {basic_monst_ai,basic_monst_ai};
int main() {
	srand(time(NULL));
	shuffle_rand_items();
	display_generic_specific();
	printf("%s",ANSI_clr);
	printc("Welcome to YACrogue!\nMade by Vijay Shanmugam and Joshua Piety\nCollect the mighty Amulet of John Doe from the 26th floor of the dungeon!\nHave fun! (press a key to start)",GREEN);
	getch_(0);
	fill_tiles(0,0,D_WIDTH,D_HEIGHT,T_AIR);
	// placeholder for dungeon gen
	dungeon[6][6] = T_LAVA;
	items[7][7] = I_ARMOR_1;
	items[7][8] = I_POTION_GREEN;
	items[8][7] = I_ARMOR_2;
	items[8][8] = I_POTION_RED;
	items[9][9] = I_DEBUG_GE;
	monsters[8][9] = MONSTER_BASIC;
	bool game_running = true;
	player_x = 1;
	player_y = 1;
	pr_info("Welcome to INROGUE!");
	while (game_running) {
		enforce_borders();
		printf("%s",ANSI_clr);
		// display health bar
		int num_dash = 10*((double)hp/(double)maxhp);
		int barcol;
		switch (num_dash) {
		case 0:
		case 1:
		case 2:
			barcol = MAGENTA;
			break;
		case 3:
		case 4:
		case 5:
			barcol = RED;
			break;
		case 6:
		case 7:
		case 8:
			barcol = YELLOW;
			break;
		case 9:
		case 10:
			barcol = GREEN;
		}
		printf("%s", ANSI_home);
		printf("HP: |");
		color_set(barcol);
		printf("%s%s",times("-",num_dash),times(" ",10-num_dash));
		color_reset();
		printf("|\n");
		display_dungeon(0,0);
		display_items(items);
		display_monsts(monsters);
		display_player(); // display player AFTER items, so it shows as @ when you're on an item
		// print messages
		for (int i = 0; i < 5; i++) {
			printf("%s\n",infos[i]);
		}
		if (important_msg) {
			getch_(0);
			important_msg = false;
		}
		char keypress = getch_(0);
		if (death) {
			;
		} else if (keypress == 'p') { // pick up items
			if (items[player_y][player_x]) {
				add_item(items[player_y][player_x],1);
				items[player_y][player_x] = I_NOTHING;
			} else {
				pr_info("You see nothing to pick up.");
			}
		} else if (keypress == 'u') { // use items
			take_inventory(1);
			printf("%sWhich item number? ",ANSI_home);
			int inum;
			scanf("%d",&inum);
			if (inum > num_pack_slots || inum < 1) {
				pr_info("You don't have that many items.");
			} else {
				inum--;
				int item = pack_items[inum];
				if (item >= FIRST_GENERICS[0] && item < FIRST_GENERICS[0]+NUM_OF_IDEN_ITEMS) { // it is a generic
					item = iden_specifics[is_contained_in(iden_generics,NUM_OF_IDEN_ITEMS,item)-1]; // make it a specific just this once
				}
				if (item == I_POTION_NOTHING) {
					pr_info("Nothing happens.");
					del_item(pack_items[inum],1);
				} else if (item == I_POTION_DEATH) {
					pr_info("You suddenly feel like you shouldn't have drunk that.");
					death = 1; // die
					death_reason = "drinking something you shouldn't have";
				} else if (item >= I_ARMOR_1 && item <= I_ARMOR_3) {
					if (armor_on) add_item(I_ARMOR_1-1+armor_on,1);
					armor_on = pack_items[inum]-I_ARMOR_1+1;
					del_item(pack_items[inum],1);
				} else if (item >= I_WEAPON_1 && item <= I_WEAPON_3) {
					if (weapon_on) add_item(I_WEAPON_1-1+weapon_on,1);
					weapon_on = pack_items[inum]-I_WEAPON_1+1;
					del_item(pack_items[inum],1);
				} else {
					char buffer[255];
					sprintf(buffer,"%s? You can't use that!",str_format(ITEM_NAMES[pack_items[inum]],1,pack_counts[inum])); // use pack_items[inum] to avoid disclosing information
					pr_info(buffer);
				}
			}
		} else if (keypress == 'w') { // view wielding and wearing
			printf("%sWearing:\n%s%s\nWearing:\n%s%s\n",ANSI_home,armor_on?ITEM_NAMES[I_ARMOR_1+armor_on-1]:"nothing",(playerStr>=ARMOR_SS[armor_on])?"":" (heavy)",weapon_on?ITEM_NAMES[I_WEAPON_1+weapon_on-1]:"nothing",(playerStr>=WEAPON_SS[weapon_on])?"":" (heavy)");
			getch_(0);
		} else if (keypress == 'i') { // take inventory
			take_inventory(0);
			printf("(press any key)");
			getch_(0);
		} else if (keypress == '\x1b') { // appears to be an ANSI escape sequence
			keypress = getch_(0);
			if (keypress == '\x5b') { // which it is!
				keypress = getch_(0); // meat of the escape sequence
				switch (keypress) {
					case 'A': // up arrow
					if (monsters[player_y-1][player_x]) mdamages[player_y-1][player_x] += attack(playerStr, MONST_STRS[monsters[player_y-1][player_x]], "YOU SHOULD NEVER SEE THIS. IF YOU DO THERE IS A GLITCH IN THE GAME",1);
					else if (!tile_flag(player_x, player_y - 1, SOLID)) player_y--;
					break;
					case 'B': // down arrow
					if (monsters[player_y+1][player_x]) mdamages[player_y-1][player_x] += attack(playerStr, MONST_STRS[monsters[player_y+1][player_x]], "YOU SHOULD NEVER SEE THIS. IF YOU DO THERE IS A GLITCH IN THE GAME",1);
					else if (!tile_flag(player_x, player_y + 1, SOLID)) player_y++;
					break;
					case 'C': // right arrow
					if (monsters[player_y][player_x+1]) mdamages[player_y][player_x+1] += attack(playerStr, MONST_STRS[monsters[player_y][player_x+1]], "YOU SHOULD NEVER SEE THIS. IF YOU DO THERE IS A GLITCH IN THE GAME",1);
					else if (!tile_flag(player_x + 1, player_y, SOLID)) player_x++;
					break;
					case 'D': // left arrow
					if (monsters[player_y][player_x-1]) mdamages[player_y][player_x-1] += attack(playerStr, MONST_STRS[monsters[player_y][player_x-1]], "YOU SHOULD NEVER SEE THIS. IF YOU DO THERE IS A GLITCH IN THE GAME",1);
					else if (!tile_flag(player_x - 1, player_y, SOLID)) player_x--;
					break;
				}
			} else if (keypress == '\x1b') { // ...or the user pressed ESC
				printc("\nWould you like to quit? type @ to confirm: ",RED);
				keypress = getch_(1);
				if (keypress == '@') {
					game_running = false;
					printc("\nOh, well. Bye!\n",RED);
				} else {
					printc("\nReally? NO? Wimp!\n",RED);
				}
				getch_(0);
			}
		}
		// check what you're standing on
		int tile_on = dungeon[player_y][player_x];
		if (!death) {
			if (tile_on == T_LAVA) {
				// burn to death
				death = 1;
				pr_info("You burned in lava.");
				death_reason = "burning in lava";
			}
		}	

		// check if there's an item below you
		if (items[player_y][player_x]) {
			char *iname = ITEM_NAMES[items[player_y][player_x]];
			char tmp_s[50];
			sprintf(tmp_s,"There is %s below you.",str_format(iname,1,1));
			pr_info(tmp_s);
		}
		// check if you've died
		if (death == 3 ) {
			game_running = false;
		} else if (death == 2) {
			death = 3;
			pr_info("You died.");
			important_msg = true;
		} else if (death == 1 || hp <= 0) { // death is for abnormal deaths
			death = 2;
		}
		// update monster ais
		for (int i = 0; i < D_HEIGHT; i++) {
			for (int j = 0; j < D_WIDTH; j++) {
				int x = monsters[i][j];
				if (x) {
					(*MONST_AIS[x])(j,i); // function pointers are weird
				}
			}
		}
	}
	if (death) { // did you die, or did the user quit?
		printf("%s",ANSI_clr);
		color_set(RED);
		printf("GAME OVER\nYou died on level POOP of the dungeon.\nYou were killed by %s.\nPress any key to exit the game\n",death_reason);
		color_reset();
		getch_(0);
	}
}
