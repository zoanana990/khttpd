#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/fs.h>
#include <linux/kthread.h>
#include <linux/sched/signal.h>
#include <linux/tcp.h>

#include "http_parser.h"
#include "http_server.h"

#define CRLF "\r\n"

/* Send Message */
#define SEND_HTTP_MESSAGE(socket, buf, format, ...)       \
    snprintf(buf, SEND_BUFFER_SIZE, format, __VA_ARGS__); \
    http_server_send(socket, buf, strlen(buf))

#define BUFFER_SIZE 256
#define RECV_BUFFER_SIZE 4096
#define SEND_BUFFER_SIZE BUFFER_SIZE
#define REQUEST_URL_SIZE BUFFER_SIZE

#define URL "/home/khienh/linux_kernel/khttpd"
#define URL_LEN strlen(URL)

extern struct workqueue_struct *khttpd_wq;

struct http_request {
    struct socket *socket;
    enum http_method method;
    char request_url[REQUEST_URL_SIZE];
    struct dir_context directory;
    int complete;
};

static int http_server_recv(struct socket *sock, char *buf, size_t size)
{
    struct kvec iov = {.iov_base = (void *) buf, .iov_len = size};
    struct msghdr msg = {.msg_name = 0,
                         .msg_namelen = 0,
                         .msg_control = NULL,
                         .msg_controllen = 0,
                         .msg_flags = 0};
    return kernel_recvmsg(sock, &msg, &iov, 1, size, msg.msg_flags);
}

static int http_server_send(struct socket *sock, const char *buf, size_t size)
{
    struct msghdr msg = {.msg_name = NULL,
                         .msg_namelen = 0,
                         .msg_control = NULL,
                         .msg_controllen = 0,
                         .msg_flags = 0};
    int done = 0;
    while (done < size) {
        struct kvec iov = {
            .iov_base = (void *) ((char *) buf + done),
            .iov_len = size - done,
        };
        int length = kernel_sendmsg(sock, &msg, &iov, 1, iov.iov_len);
        if (length < 0) {
            pr_err("write error: %d\n", length);
            break;
        }
        done += length;
    }
    return done;
}

static int directory_traversal(struct dir_context *directory,
                               const char *name,
                               int namelen,
                               loff_t offset,
                               u64 ino,
                               unsigned int d_type)
{
    if (strcmp(name, ".")) {
        char buf[SEND_BUFFER_SIZE] = {0};
        struct http_request *request =
            container_of(directory, struct http_request, directory);

        SEND_HTTP_MESSAGE(
            request->socket, buf,
            "%lx" CRLF "<tr><td><a href=\"%s%s\">%s</a></td></tr>" CRLF,
            34 + (strlen(name) << 1) + strlen(request->request_url),
            request->request_url, name, name);
    } else if (!strcmp(name, "..")) {
        /* Go back to the last directory */
    }
    return 0;
}

static int directory_listing(struct http_request *request)
{
    struct file *fp;
    char buf[SEND_BUFFER_SIZE]; /* store the information we will send */
    char current_directory[REQUEST_URL_SIZE]; /* store the current directory,
                                                 that is pwd */

    /* Initialize buf and current_directory */
    memset(buf, 0, SEND_BUFFER_SIZE);
    memset(current_directory, 0, REQUEST_URL_SIZE);

    request->directory.actor = directory_traversal;

    /* Copy root position to current_directory */
    /* Concatenation */
    memcpy(current_directory, URL, URL_LEN);
    memcpy(current_directory + URL_LEN, request->request_url,
           strlen(request->request_url));

    pr_info(MODULE_NAME ": current_directory = %s", current_directory);
    pr_info(MODULE_NAME ": request->request_url = %s", request->request_url);

    fp = filp_open(current_directory, O_RDONLY, 0);

    if (IS_ERR(fp)) {
        SEND_HTTP_MESSAGE(
            request->socket, buf, "%s%s%s", "HTTP/1.1 404 Not Found" CRLF,
            "Content-Type: text/plain" CRLF "Content-Length: 13\r\n" CRLF,
            "Connection: Close" CRLF CRLF "404 Not Found");
        return 1;
    }

    if (S_ISDIR(fp->f_inode->i_mode)) {
        /* Open folder */
        SEND_HTTP_MESSAGE(request->socket, buf, "%s%s%s",
                          "HTTP/1.1 200 OK" CRLF,
                          "Content-Type: text/html" CRLF,
                          "Transfer-Encoding: chunked" CRLF CRLF);
        SEND_HTTP_MESSAGE(request->socket, buf, "7B" CRLF "%s%s%s%s",
                          "<html><head><style>" CRLF,
                          "body{font-family: monospace; font-size: 15px;}" CRLF,
                          "td {padding: 1.5px 6px;}" CRLF,
                          "</style></head><body><table>" CRLF);

        iterate_dir(fp, &request->directory);

        SEND_HTTP_MESSAGE(request->socket, buf, "%s",
                          "16" CRLF "</table></body></html>" CRLF);
        SEND_HTTP_MESSAGE(request->socket, buf, "%s", "0" CRLF CRLF);

    } else if (S_ISREG(fp->f_inode->i_mode)) {
        /* Open File */
        char *read_data = kmalloc(fp->f_inode->i_size, GFP_KERNEL);
        int ret = kernel_read(fp, read_data, fp->f_inode->i_size, 0);

        /* Open the file depends on the corresponding file type */
        // strncat(request->request_url, path, strlen(path));
        SEND_HTTP_MESSAGE(request->socket, buf, "%s%d%s",
                          "HTTP/1.1 200 OK" CRLF "Content-Type: plain/text" CRLF
                          "Content-Length: ",
                          ret, CRLF "Connection: Close" CRLF CRLF);

        http_server_send(request->socket, read_data, strlen(read_data));
        kfree(read_data);
    }

    filp_close(fp, NULL);

    return 0;
}

static int http_server_response(struct http_request *request, int keep_alive)
{
    if (request->method != HTTP_GET) {
        char buf[SEND_BUFFER_SIZE] = {0};
        SEND_HTTP_MESSAGE(request->socket, buf, "%s",
                          "HTTP/1.1 501 Not Implemented" CRLF
                          "Content-Type: text/plain" CRLF
                          "Content-Length: 19" CRLF
                          "Connection: Close" CRLF CRLF "501 Not Implemented");
        return 1;
    }
    pr_info("url = %s", request->request_url);
    if (directory_listing(request)) {
        pr_info(MODULE_NAME ": Directory Listing Failed\n");
        return 1;
    }
    return 0;
}

static int http_parser_callback_message_begin(http_parser *parser)
{
    struct http_request *request = parser->data;
    struct socket *socket = request->socket;
    memset(request, 0x00, sizeof(struct http_request));
    request->socket = socket;
    return 0;
}

static int http_parser_callback_request_url(http_parser *parser,
                                            const char *p,
                                            size_t len)
{
    // if (len > (REQUEST_URL_SIZE - 1))
    //     len %= (REQUEST_URL_SIZE - 1);

    // struct http_request *request = parser->data;
    // size_t l = strlen(request->request_url);

    // if (l + len > (REQUEST_URL_SIZE - 1)) {
    //     len = (REQUEST_URL_SIZE - 1) - l;
    // }

    // strncat(request->request_url, p, len);
    // return 0;
    struct http_request *request = parser->data;
    strncat(request->request_url, p, len);
    return 0;
}

static int http_parser_callback_header_field(http_parser *parser,
                                             const char *p,
                                             size_t len)
{
    return 0;
}

static int http_parser_callback_header_value(http_parser *parser,
                                             const char *p,
                                             size_t len)
{
    return 0;
}

static int http_parser_callback_headers_complete(http_parser *parser)
{
    struct http_request *request = parser->data;
    request->method = parser->method;
    return 0;
}

static int http_parser_callback_body(http_parser *parser,
                                     const char *p,
                                     size_t len)
{
    return 0;
}

static int http_parser_callback_message_complete(http_parser *parser)
{
    struct http_request *request = parser->data;
    http_server_response(request, http_should_keep_alive(parser));
    request->complete = 1;
    return 0;
}

static void http_server_worker(struct work_struct *work)
{
    struct khttpd *worker = container_of(work, struct khttpd, khttpd_work);
    char *buf;

    struct http_parser parser;
    struct http_parser_settings setting = {
        .on_message_begin = http_parser_callback_message_begin,
        .on_url = http_parser_callback_request_url,
        .on_header_field = http_parser_callback_header_field,
        .on_header_value = http_parser_callback_header_value,
        .on_headers_complete = http_parser_callback_headers_complete,
        .on_body = http_parser_callback_body,
        .on_message_complete = http_parser_callback_message_complete};
    struct http_request request;

    allow_signal(SIGKILL);
    allow_signal(SIGTERM);

    buf = kmalloc(RECV_BUFFER_SIZE, GFP_KERNEL);
    if (!buf) {
        pr_err(MODULE_NAME ": can't allocate memory!\n");
        return;
    }

    request.socket = worker->sock;
    http_parser_init(&parser, HTTP_REQUEST);
    parser.data = &request;
    while (!daemon.is_stopped) {
        int ret = http_server_recv(worker->sock, buf, RECV_BUFFER_SIZE - 1);
        if (ret <= 0) {
            if (ret)
                printk(KERN_ERR MODULE_NAME ": recv error: %d\n", ret);
            break;
        }
        http_parser_execute(&parser, &setting, buf, ret);
        if (request.complete && !http_should_keep_alive(&parser))
            break;
    }
    kernel_sock_shutdown(worker->sock, SHUT_RDWR);
    kfree(buf);
}

static struct work_struct *create_work(struct socket *sk)
{
    struct khttpd *work;

    if (!(work = kmalloc(sizeof(struct khttpd), GFP_KERNEL)))
        return NULL;

    work->sock = sk;

    INIT_WORK(&work->khttpd_work, http_server_worker);

    list_add(&work->list, &daemon.worker);

    return &work->khttpd_work;
}

static void free_work(void)
{
    struct khttpd *l, *tar;
    /* cppcheck-suppress uninitvar */

    list_for_each_entry_safe (tar, l, &daemon.worker, list) {
        kernel_sock_shutdown(tar->sock, SHUT_RDWR);
        flush_work(&tar->khttpd_work);
        sock_release(tar->sock);
        kfree(tar);
    }
}

int http_server_daemon(void *arg)
{
    struct socket *socket;
    struct work_struct *worker;
    struct http_server_param *param = (struct http_server_param *) arg;

    allow_signal(SIGKILL);
    allow_signal(SIGTERM);

    INIT_LIST_HEAD(&daemon.worker);

    while (!kthread_should_stop()) {
        int err = kernel_accept(param->listen_socket, &socket, 0);
        if (err < 0) {
            if (signal_pending(current))
                break;
            pr_err("kernel_accept() error: %d\n", err);
            continue;
        }
        if (unlikely(!(worker = create_work(socket)))) {
            printk(KERN_ERR MODULE_NAME
                   ": create work error, connection closed\n");
            kernel_sock_shutdown(socket, SHUT_RDWR);
            sock_release(socket);
            continue;
        }
        queue_work(khttpd_wq, worker);
    }

    daemon.is_stopped = true;
    free_work();

    return 0;
}
