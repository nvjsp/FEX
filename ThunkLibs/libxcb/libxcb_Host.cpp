/*
$info$
tags: thunklibs|xcb
$end_info$
*/

#include <chrono>
#include <cstring>
#include <malloc.h>
#include <thread>
#include <stdio.h>
#include <unistd.h>
#include <syscall.h>
#include <sys/syscall.h>

#include <xcb/xcb.h>
#include <xcb/xcbext.h>

#include "common/CrossArchEvent.h"
#include "common/Host.h"
#include <dlfcn.h>
#include <unordered_map>

#include "callback_structs.inl"
#include "WorkEventData.h"
#include "callback_typedefs.inl"

struct {
    #include "callback_unpacks_header.inl"
} *callback_unpacks;

#include "ldr_ptrs.inl"

static void fexfn_impl_libxcb_FEX_xcb_init_extension(xcb_connection_t*, xcb_extension_t*);
static size_t fexfn_impl_libxcb_FEX_usable_size(void*);
static void fexfn_impl_libxcb_FEX_free_on_host(void*);
static void fexfn_impl_libxcb_FEX_GiveEvents(CrossArchEvent*, CrossArchEvent*, CBWork*);

static int fexfn_impl_libxcb_xcb_take_socket(xcb_connection_t * a_0, fex_guest_function_ptr a_1, void * a_2, int a_3, uint64_t * a_4);

#include "function_unpacks.inl"

struct xcb_take_socket_CB_args {
  xcb_connection_t * conn;
  fex_guest_function_ptr CBFunction;
  void *closure;
};

CrossArchEvent *WaitForWork{};
CrossArchEvent *WorkDone{};
CBWork *Work{};

static std::unordered_map<xcb_connection_t*, xcb_take_socket_CB_args> CBArgs{};

static size_t fexfn_impl_libxcb_FEX_usable_size(void *a_0){
  return malloc_usable_size(a_0);
}

static void fexfn_impl_libxcb_FEX_free_on_host(void *a_0){
  free(a_0);
}

static void fexfn_impl_libxcb_FEX_GiveEvents(CrossArchEvent* a_0, CrossArchEvent* a_1, CBWork* a_2){
  WaitForWork = a_0;
  WorkDone = a_1;
  Work = a_2;
}

static void xcb_take_socket_cb(void *closure) {
  xcb_take_socket_CB_args *Args = (xcb_take_socket_CB_args *)closure;

  // Signalling to the guest thread like this allows us to call the callback function from any thread without
  // creating spurious thread objects inside of FEX
  Work->cb = Args->CBFunction;
  Work->argsv = Args->closure;
  // Tell the thread it has work
  NotifyWorkFunc(WaitForWork);
  // Wait for the work to be done
  WaitForWorkFunc(WorkDone);
}

static int fexfn_impl_libxcb_xcb_take_socket(xcb_connection_t * a_0, fex_guest_function_ptr a_1, void * a_2, int a_3, uint64_t * a_4){
  xcb_take_socket_CB_args Args{};
  Args.conn = a_0;
  Args.CBFunction = a_1;
  Args.closure = a_2;

  auto Res = CBArgs.insert_or_assign(a_0, Args);

  return fexldr_ptr_libxcb_xcb_take_socket
    (a_0,
     xcb_take_socket_cb,
     &Res.first->second,
     a_3,
     a_4);
}

static ExportEntry exports[] = {
    #include "tab_function_unpacks.inl"
    { nullptr, nullptr }
};

#include "ldr.inl"

static void fexfn_impl_libxcb_FEX_xcb_init_extension(xcb_connection_t * a_0, xcb_extension_t * a_1){
  xcb_extension_t *ext{};

  if (strcmp(a_1->name, "BIG-REQUESTS") == 0) {
    ext = (xcb_extension_t *)dlsym(fexldr_ptr_libxcb_so, "xcb_big_requests_id");
  }
  else if (strcmp(a_1->name, "XC-MISC") == 0) {
    ext = (xcb_extension_t *)dlsym(fexldr_ptr_libxcb_so, "xcb_xc_misc_id");
  }
  else {
    fprintf(stderr, "Unknown xcb extension '%s'\n", a_1->name);
    __builtin_trap();
    return;
  }

  if (!ext) {
    fprintf(stderr, "Couldn't find extension symbol: '%s'\n", a_1->name);
    __builtin_trap();
    return;
  }
  [[maybe_unused]] auto res = fexldr_ptr_libxcb_xcb_get_extension_data(a_0, ext);

  // Copy over the global id
  a_1->global_id = ext->global_id;
}

EXPORTS_WITH_CALLBACKS(libxcb)
