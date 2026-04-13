#define NOTIF_HEIGHT 20
#define NOTIF_BACKGROUND (Color){.r = 209, .g = 227, .b = 255, .a = 230}
#define NOTIF_FOREGROUND (Color){.r = 7, .g = 17, .b = 33, .a = 255}

void notifications_push(Notification notification)
{
    arrins(notification_queue.list, 0, notification);
}

void notifications_update(void)
{
    for (s32 i = arrlen(notification_queue.list) - 1; i >= 0; i--) {
        Notification *n = &notification_queue.list[i];
        n->lifetime -= renderer.dt;
        if (n->lifetime <= 0) {
            arrdel(notification_queue.list, i);
        }
    }
}

void notifications_flush(Font *font)
{
    s32 len = arrlen(notification_queue.list);
    if (len == 0) return;

    s32 padding = 4;
    s32 item_h  = NOTIF_HEIGHT + padding;

    // Stack grows upward from center
    s32 total_h = len * item_h;
    s32 start_y = (GAME_HEIGHT / 2) - (total_h / 2);

    for (s32 i = 0; i < len; i++) {
        Notification *n = &notification_queue.list[i];

        s32 text_w = n->msg.len * 16;
        s32 box_w  = text_w + padding * 2;
        s32 x      = (GAME_WIDTH / 2) - (box_w / 2);
        s32 y      = start_y + i * item_h;

        draw_recs32((Recs32){.x = x, .y = y, .w = box_w, .h = NOTIF_HEIGHT}, 0.1, NOTIF_BACKGROUND);
        draw_string8(font, n->msg, v2i(x + padding, y + padding), 16, NOTIF_FOREGROUND);
    }
}
