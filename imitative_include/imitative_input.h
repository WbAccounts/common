

#define EV_KEY			0x01
#define EV_REL			0x02
#define EV_ABS			0x03

#define EVIOCGRAB		_IOW('E', 0x90, int)			/* Grab/Release device */

struct libinput;
struct libinput_interface;
struct libinput_log_handler;
struct libinput_event;
struct libinput_device;

struct libinput_interface {
	/**
	 * Open the device at the given path with the flags provided and
	 * return the fd.
	 *
	 * @param path The device path to open
	 * @param flags Flags as defined by open(2)
	 * @param user_data The user_data provided in
	 * libinput_udev_create_context()
	 *
	 * @return The file descriptor, or a negative errno on failure.
	 */
	int (*open_restricted)(const char *path, int flags, void *user_data);
	/**
	 * Close the file descriptor.
	 *
	 * @param fd The file descriptor to close
	 * @param user_data The user_data provided in
	 * libinput_udev_create_context()
	 */
	void (*close_restricted)(int fd, void *user_data);
};

enum libinput_event_type {
    LIBINPUT_EVENT_KEYBOARD_KEY = 300,

	LIBINPUT_EVENT_POINTER_MOTION = 400,
	LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE,
	LIBINPUT_EVENT_POINTER_BUTTON,
	LIBINPUT_EVENT_POINTER_AXIS,

	LIBINPUT_EVENT_TOUCH_DOWN = 500,
	LIBINPUT_EVENT_TOUCH_UP,
	LIBINPUT_EVENT_TOUCH_MOTION,
	LIBINPUT_EVENT_TOUCH_CANCEL,
	/**
	 * Signals the end of a set of touchpoints at one device sample
	 * time. This event has no coordinate information attached.
	 */
	LIBINPUT_EVENT_TOUCH_FRAME,
};
