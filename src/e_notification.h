#ifndef NOTIFICATION_H
#define NOTIFICATION_H

typedef enum {
    NOTIFICATION_ERROR,
    NOTIFICATION_WARNING,
    NOTIFICATION_INFO,
    NOTIFICATION_COUNT,
} NotificationTag;

typedef struct {
    NotificationTag tag;
    s32     time_in_ms;
    String8 msg;
} Notification;

static struct {
    f32 delta_time;
    Notification *list;
} notification_queue = {
    .delta_time = 0,
    .list = NULL,
};

void notifications_push(Notification notification);
void notifications_update(void);
void notifications_flush(void);

#endif // NOTIFICATION_H
