#include <string.h>
#include <iostream>

#include "dart_dll.h"

#include <dart_api.h>

#define ENSURE(X)                                                              \
  if (!(X)) {                                                                  \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, "Check failed: " #X);   \
    exit(1);                                                                   \
  }

#define ENSURE_VALID(X) ENSURE(!Dart_IsError(X))

Dart_Handle HandleError(Dart_Handle handle) {
  if (Dart_IsError(handle)) {
    Dart_PropagateError(handle);
  }
  return handle;
}

void Generate(Dart_NativeArguments arguments) {
  int num_args = Dart_GetNativeArgumentCount(arguments);

  // Get the named arguments
  Dart_Handle query = HandleError(Dart_GetNativeArgument(arguments, 0));
  Dart_Handle model = HandleError(Dart_GetNativeArgument(arguments, 1));

  // Convert arguments to C strings
  const char* query_str = nullptr;
  const char* model_str = nullptr;

  if (Dart_IsString(query)) {
    HandleError(Dart_StringToCString(query, &query_str));
    std::cout << "Query: " << query_str << std::endl;
  }

  if (Dart_IsString(model)) {
    HandleError(Dart_StringToCString(model, &model_str));
    std::cout << "Model: " << model_str << std::endl;
  }

  // Create a mock response
  std::string response = "Generated response for query: ";
  response += query_str ? query_str : "null";
  response += " using model: ";
  response += model_str ? model_str : "null";

  Dart_SetReturnValue(arguments, Dart_NewStringFromCString(response.c_str()));
}

Dart_NativeFunction ResolveNativeFunction(Dart_Handle name,
                                          int num_of_arguments,
                                          bool* auto_setup_scope) {
  ENSURE(Dart_IsString(name));

  Dart_NativeFunction result = nullptr;

  const char* name_str;
  ENSURE_VALID(Dart_StringToCString(name, &name_str));

  if (strcmp(name_str, "AISdk.generate") == 0) {
    result = Generate;
  }

  return result;
}

int main() {
  // Initialize Dart
  DartDllConfig config;
  config.service_port = 6222;

  if (!DartDll_Initialize(config)) return -1;

  // Load your main isolate file, also providing the path to a package config if one exists.
  // The build makes sure these are coppied to the output directory for running the example
  Dart_Isolate isolate = DartDll_LoadScript("hello_world.dart", nullptr);
  if (isolate == nullptr) return -1;

  // With the library loaded, you can now use the dart_api.h functions
  // This includes setting the native function resolver:
  Dart_EnterIsolate(isolate);
  Dart_EnterScope();

  Dart_Handle library = Dart_RootLibrary();
  Dart_SetNativeResolver(library, ResolveNativeFunction, nullptr);

  Dart_Handle mainFunction =
      HandleError(Dart_GetField(library, Dart_NewStringFromCString("main")));

  // And run "main"
  Dart_Handle result = DartDll_RunMain(library);
  if (Dart_IsError(result)) {
    std::cerr << Dart_GetError(result);
  }

  Dart_ExitScope();
  Dart_ShutdownIsolate();

  // Don't forget to shutdown
  DartDll_Shutdown();

  return 0;
}
