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
    f32     lifetime;
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
void notifications_update(f32 delta_time);
void notifications_flush(Font *font);

#endif // NOTIFICATION_H
