/*
 * EXAMPLE:
 *
 *  var inputEventListener = require('../build/Release/input_event_listener.node').inputEventListener
 *
 *  inputEventListener("/dev/input/event1", function(event){
 *        console.log(event); // event has fields `type` `code` `value` `timestamp` in seconds;
 *  });
 *
 *
 */
#include <unistd.h>
#define Sleep(x) usleep((x)*1000)
#include <nan.h>
#include <poll.h>
#include <linux/input.h>
#include <queue>

bool LOOP_STARTED;

class ProgressWorker : public NanAsyncProgressWorker {
    public:
    ProgressWorker(NanCallback *progress, std::string filename_):
    NanAsyncProgressWorker(progress),
    progress(progress),
    filename(filename_) {
        uv_mutex_init(&queue_lock);
    }

    ~ProgressWorker() {
        uv_mutex_destroy(&queue_lock);
    }

    void Execute (const NanAsyncProgressWorker::ExecutionProgress& progress) {
        input_event event;
        struct pollfd  fds[1];

        fds[0].fd = open (filename.c_str(), O_RDONLY );
        fds[0].events = POLLIN;

        while(LOOP_STARTED) {
            if ( poll(fds, 1, 100) > 0 ){
                read(fds[0].fd, (char *)&event, sizeof(input_event));
                uv_mutex_lock(&queue_lock);
                event_queue.push (event);
                uv_mutex_unlock(&queue_lock);
                progress.Send(NULL, 0);
            }
        }
    }

    void HandleProgressCallback(const char *data, size_t size) {
        NanScope();

        while (!event_queue.empty()) {
            uv_mutex_lock(&queue_lock);
				input_event event = event_queue.front();
				event_queue.pop();
				v8::Local<v8::Object> result = NanNew<v8::Object>();
				result->Set(NanNew<v8::String>("type"), NanNew<v8::Integer>(event.type));
				result->Set(NanNew<v8::String>("code"), NanNew<v8::Integer>(event.code));
				result->Set(NanNew<v8::String>("value"), NanNew<v8::Integer>(event.value));
				result->Set(NanNew<v8::String>("timestamp"), NanNew<v8::Number>(event.time.tv_sec ));
				v8::Local<v8::Value> argv[] = {result};
				progress->Call(1, argv);
            uv_mutex_unlock(&queue_lock);
        }
    }

    private:
    std::string filename;
    NanCallback *progress;
    uv_mutex_t queue_lock;
    std::queue<input_event> event_queue;
};

NAN_METHOD(InputEventListener) {
    NanScope();
    LOOP_STARTED = true;
    std::string filename(*NanAsciiString(args[0]));
    NanCallback *progress = new NanCallback(args[1].As<v8::Function>());
    NanAsyncQueueWorker(new ProgressWorker(progress, filename));
    NanReturnUndefined();
}

NAN_METHOD(Destroy) {
    NanScope();
    LOOP_STARTED = false;
    NanReturnUndefined();
}

void Init(v8::Handle<v8::Object> exports) {
    exports->Set(
    NanNew<v8::String>("inputEventListener")
    , NanNew<v8::FunctionTemplate>(InputEventListener)->GetFunction());

    exports->Set(
    NanNew<v8::String>("destroy")
    , NanNew<v8::FunctionTemplate>(Destroy)->GetFunction());
}

NODE_MODULE(asyncprogressworker, Init)
