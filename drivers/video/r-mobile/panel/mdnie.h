#ifndef __MDNIE_H__
#define __MDNIE_H__

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

/* #define END_SEQ			0xffff*/
#define END_SEQ				0xff

enum MODE {
	DYNAMIC,
	STANDARD,
	NATURAL,
	MOVIE,
	AUTO,
	MODE_MAX
};

enum SCENARIO {
	UI_MODE,
	VIDEO_MODE,
	CAMERA_MODE = 4,
	NAVI_MODE,
	GALLERY_MODE,
	VT_MODE,
	BROWSER_MODE,
	EBOOK_MODE,
	EMAIL_MODE,
	SCENARIO_MAX,
	COLOR_TONE_1 = 40,
	COLOR_TONE_2,
	COLOR_TONE_3,
	COLOR_TONE_MAX	
};

enum SCENARIO_DMB {
	DMB_NORMAL_MODE = 20,
	DMB_MODE_MAX
};

enum CABC {
	CABC_OFF,
	CABC_ON,
	CABC_MAX,
};

enum POWER_LUT {
	LUT_DEFAULT,
	LUT_VIDEO,
	LUT_MAX,
};

enum POWER_LUT_LEVEL {
	LUT_LEVEL_MANUAL_AND_INDOOR,
	LUT_LEVEL_OUTDOOR_1,
	LUT_LEVEL_OUTDOOR_2,
	LUT_LEVEL_MAX,
};

enum ACCESSIBILITY {
	ACCESSIBILITY_OFF,
	NEGATIVE,
	COLOR_BLIND,
	ACCESSIBILITY_MAX
};

enum NEGATIVE {
	NEGATIVE_OFF,
	NEGATIVE_ON,
	NEGATIVE_MAX,
};

struct mdnie_tunning_info {
	char *name;
	const unsigned short *seq;
};

struct mdnie_tunning_info_cabc {
	char *name;
	const unsigned short *seq;
	unsigned int idx_lut;
};

struct mdnie_info {
	struct device			*dev;
	struct mutex			lock;
	struct mutex			dev_lock;

	unsigned int enable;
	enum SCENARIO scenario;
	enum MODE mode;
	enum CABC cabc;
	unsigned int tunning;
	unsigned int negative;	
	unsigned int accessibility;
	unsigned int color_correction;
	char path[50];
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
};

struct specific_cmdset {
	unsigned char cmd;
	unsigned char *data;
	int size;
};

extern struct mdnie_info *g_mdnie;

int mdnie_send_sequence(struct mdnie_info *mdnie,
			const struct specific_cmdset *cmdset);
extern void set_mdnie_value(struct mdnie_info *mdnie, u8 force);
#if defined(CONFIG_FB_MDNIE_PWM)
extern void set_mdnie_pwm_value(struct mdnie_info *mdnie, int value);
#endif
extern int mdnie_txtbuf_to_parsing(char const *pfilepath);

extern void check_lcd_type(void);
struct mdnie_backlight_value {
	unsigned int max;
	unsigned int mid;
	unsigned char low;
	unsigned char dim;
};

#endif /* __MDNIE_H__ */
