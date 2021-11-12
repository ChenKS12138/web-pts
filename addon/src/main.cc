#include "pts.h"
#include <nan.h>

using Nan::GetFunction;
using Nan::New;
using Nan::Set;
using Nan::To;
using v8::FunctionTemplate;
using v8::String;

extern char PtsErrMsg[PTS_ERR_MSG_SIZE];

NAN_METHOD(createPts) {
    int child_pid;
    int fd = create_pts(&child_pid);
    if (fd == -1)
        Nan::ThrowError(PtsErrMsg);
    Nan::HandleScope scope;
    v8::Local<v8::Object> result = New<v8::Object>();
    Set(result, New("ptsFd").ToLocalChecked(), New(fd));
    Set(result, New("childPid").ToLocalChecked(), New(child_pid));
    info.GetReturnValue().Set(result);
}

NAN_METHOD(killPtsChild) {
    if (info.Length() < 1)
        Nan::ThrowError("require fd");
    if (kill_pts_child(To<int>(info[0]).FromJust()) == -1)
        Nan::ThrowError(PtsErrMsg);
}

NAN_METHOD(resizePts) {
    if (info.Length() < 3)
        Nan::ThrowError("require fd rows cols");
    if (resize_pts(To<int>(info[0]).FromJust(), To<int>(info[1]).FromJust(),
                   To<int>(info[2]).FromJust()) == -1)
        Nan::ThrowError(PtsErrMsg);
}

NAN_MODULE_INIT(Init) {
    Set(target, New<String>("createPts").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(createPts)).ToLocalChecked());
    Set(target, New<String>("killPtsChild").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(killPtsChild)).ToLocalChecked());
    Set(target, New<String>("resizePts").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(resizePts)).ToLocalChecked());
}

NODE_MODULE(nodejs_pts, Init)