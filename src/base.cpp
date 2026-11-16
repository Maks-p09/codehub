#include <algorithm>
#include <array>
#include <chrono>
#include <cwchar>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/gl.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

namespace
{
    constexpr float HeaderHeight = 68.0f;
    constexpr float FooterHeight = 30.0f;
    constexpr float PanelGap = 10.0f;
    constexpr float SidebarWidth = 220.0f;
    constexpr std::size_t CodeBufferSize = 32768;
    constexpr std::size_t MaxOutputBytes = 256 * 1024;
    constexpr int CompileTimeoutMs = 30000;
    constexpr int RunTimeoutMs = 10000;

    enum class Language
    {
        English,
        Kazakh
    };

    enum class TextKey
    {
        Code,
        History,
        English,
        Kazakh,
        Home,
        Compiler,
        Submissions,
        Status,
        CurrentCode,
        Workspace,
        HomeHint,
        OpenEditor,
        Tasks,
        TasksEmpty,
        EditorTitle,
        SubmitSolution,
        Close,
        YourCode,
        Send,
        ChooseCompiler,
        NoCompilers,
        TabHint,
        CompilationPending,
        CurrentCodeStatus,
        NoSubmissions,
        LatestCode,
        SubmissionHistory,
        Output,
        HistoryEmpty,
        DragToClose,
        CompilerNotFound,
        EnterCode,
        Running,
        Success,
        Failed,
        TimedOut,
        CompilerLaunchFailed,
        OutputTruncated,
        NoOutput
    };

    const char* Translate(Language language, TextKey key)
    {
        if (language == Language::Kazakh) {
            switch (key) {
            case TextKey::Code: return "Код";
            case TextKey::History: return "Тарих";
            case TextKey::English: return "Ағылшынша";
            case TextKey::Kazakh: return "Қазақша";
            case TextKey::Home: return "Басты бет";
            case TextKey::Compiler: return "Компилятор";
            case TextKey::Submissions: return "Жіберілімдер";
            case TextKey::Status: return "Күйі";
            case TextKey::CurrentCode: return "Ағымдағы код";
            case TextKey::Workspace: return "ЖҰМЫС АЙМАҒЫ";
            case TextKey::HomeHint: return "Код батырмасын басып, компиляторды таңдап, шешімді жіберіңіз. "
                                          "Тарих бөлімінде соңғы нәтиже мен жіберілген кодтар көрсетіледі.";
            case TextKey::OpenEditor: return "Код редакторын ашу";
            case TextKey::Tasks: return "ТАПСЫРМАЛАР";
            case TextKey::TasksEmpty: return "Тапсырмалар осы жерде көрсетіледі.";
            case TextKey::EditorTitle: return "Кодты жіберу";
            case TextKey::SubmitSolution: return "Шешімді жіберу";
            case TextKey::Close: return "Жабу";
            case TextKey::YourCode: return "Кодыңыз";
            case TextKey::Send: return "Жіберу";
            case TextKey::ChooseCompiler: return "Компиляторды таңдаңыз";
            case TextKey::NoCompilers: return "Python немесе MSYS2 компиляторы табылмады.";
            case TextKey::TabHint: return "Tab пернесі шегініс енгізеді.";
            case TextKey::CompilationPending: return "Код компьютерде орындалады; орындалу уақыты 10 секундпен шектелген.";
            case TextKey::CurrentCodeStatus: return "АҒЫМДАҒЫ КОДТЫҢ КҮЙІ";
            case TextKey::NoSubmissions: return "Әзірше жіберілім жоқ.";
            case TextKey::LatestCode: return "Соңғы жіберілген код:";
            case TextKey::SubmissionHistory: return "ЖІБЕРІЛІМДЕР ТАРИХЫ";
            case TextKey::Output: return "НӘТИЖЕ";
            case TextKey::HistoryEmpty: return "Тарих әзірше бос.";
            case TextKey::DragToClose: return "ЖАБУ ҮШІН ОҢҒА СҮЙРЕҢІЗ";
            case TextKey::CompilerNotFound: return "Компилятор табылмады.";
            case TextKey::EnterCode: return "Алдымен код енгізіңіз.";
            case TextKey::Running: return "Код орындалып жатыр...";
            case TextKey::Success: return "Сәтті орындалды.";
            case TextKey::Failed: return "Қате: бағдарлама нөлден басқа кодпен аяқталды.";
            case TextKey::TimedOut: return "Уақыт шегі асып кетті; процесс тоқтатылды.";
            case TextKey::CompilerLaunchFailed: return "Компиляторды іске қосу мүмкін болмады.";
            case TextKey::OutputTruncated: return "Шығыс көлем шегіне байланысты қысқартылды.";
            case TextKey::NoOutput: return "Бағдарлама шығыс мәтінін шығармады.";
            }
        }

        switch (key) {
        case TextKey::Code: return "Code";
        case TextKey::History: return "History";
        case TextKey::English: return "English";
        case TextKey::Kazakh: return "Kazakh";
        case TextKey::Home: return "Home";
        case TextKey::Compiler: return "Compiler";
        case TextKey::Submissions: return "Submissions";
        case TextKey::Status: return "Status";
        case TextKey::CurrentCode: return "Current Code";
        case TextKey::Workspace: return "WORKSPACE";
        case TextKey::HomeHint: return "Select Code to choose a compiler and submit a solution. "
                                      "History shows the latest result and previous submissions.";
        case TextKey::OpenEditor: return "Open code editor";
        case TextKey::Tasks: return "TASKS";
        case TextKey::TasksEmpty: return "Tasks will appear here.";
        case TextKey::EditorTitle: return "Submit Code";
        case TextKey::SubmitSolution: return "Submit solution";
        case TextKey::Close: return "Close";
        case TextKey::YourCode: return "Your code";
        case TextKey::Send: return "Send";
        case TextKey::ChooseCompiler: return "Choose a compiler";
        case TextKey::NoCompilers: return "No Python or MSYS2 compiler was found.";
        case TextKey::TabHint: return "Press Tab to insert an indentation tab.";
        case TextKey::CompilationPending: return "Code runs locally with a 10-second execution limit.";
        case TextKey::CurrentCodeStatus: return "CURRENT CODE STATUS";
        case TextKey::NoSubmissions: return "No submissions yet.";
        case TextKey::LatestCode: return "Most recently submitted code:";
        case TextKey::SubmissionHistory: return "SUBMISSION HISTORY";
        case TextKey::Output: return "OUTPUT";
        case TextKey::HistoryEmpty: return "History is empty.";
        case TextKey::DragToClose: return "DRAG RIGHT TO CLOSE";
        case TextKey::CompilerNotFound: return "Compiler not found.";
        case TextKey::EnterCode: return "Enter code before submitting.";
        case TextKey::Running: return "Running code...";
        case TextKey::Success: return "Program finished successfully.";
        case TextKey::Failed: return "Error: program exited with a non-zero code.";
        case TextKey::TimedOut: return "Time limit exceeded; process was stopped.";
        case TextKey::CompilerLaunchFailed: return "Could not start the compiler.";
        case TextKey::OutputTruncated: return "Output truncated at the size limit.";
        case TextKey::NoOutput: return "Program produced no output.";
        }
        return "";
    }

    enum class CompilerKind
    {
        Cpp,
        Python
    };

    struct CompilerOption
    {
        std::string name;
        std::filesystem::path executable;
        CompilerKind kind;
    };

    struct ExecutionResult
    {
        int exitCode = -1;
        bool timedOut = false;
        bool couldNotStart = false;
        bool outputTruncated = false;
        std::string output;
    };

    struct Submission
    {
        int number;
        std::string compiler;
        std::string code;
        ExecutionResult result;
    };

    struct AppState
    {
        std::array<char, CodeBufferSize> codeBuffer{};
        std::vector<Submission> submissions;
        std::vector<CompilerOption> compilers;
        std::future<ExecutionResult> pendingExecution;
        std::string lastOutput;
        std::string lastCompiler;
        std::string lastCode;
        Language language = Language::English;
        TextKey lastStatus = TextKey::NoSubmissions;
        int selectedCompiler = 0;
        int nextSubmissionNumber = 1;
        int drawerTab = 0;
        float drawerOffset = 0.0f;
        bool codePanelOpen = false;
        bool drawerOpen = false;
        bool executionPending = false;
    };

    std::vector<CompilerOption> FindCompilers()
    {
        std::vector<CompilerOption> compilers;
#ifdef _WIN32
        const char* msysRootEnv = std::getenv("MSYS2_ROOT");
        const std::filesystem::path msysRoot =
            msysRootEnv != nullptr ? msysRootEnv : "C:\\msys64";
        const std::array<std::pair<const char*, std::filesystem::path>, 3> cppCandidates{{
            {"MSYS2 UCRT64 (C++20)", msysRoot / "ucrt64" / "bin" / "g++.exe"},
            {"MSYS2 CLANG64 (C++20)", msysRoot / "clang64" / "bin" / "clang++.exe"},
            {"MSYS2 MINGW64 (C++20)", msysRoot / "mingw64" / "bin" / "g++.exe"}
        }};
        for (const auto& candidate : cppCandidates) {
            std::error_code error;
            if (std::filesystem::is_regular_file(candidate.second, error)) {
                compilers.push_back({candidate.first, candidate.second, CompilerKind::Cpp});
            }
        }
        for (const auto& compiler : {std::pair<const wchar_t*, CompilerKind>{L"g++.exe", CompilerKind::Cpp},
                                     {L"clang++.exe", CompilerKind::Cpp}}) {
            std::array<wchar_t, MAX_PATH> compilerPath{};
            if (SearchPathW(nullptr, compiler.first, nullptr,
                            static_cast<DWORD>(compilerPath.size()), compilerPath.data(), nullptr) > 0) {
                const std::filesystem::path executable(compilerPath.data());
                const bool alreadyFound = std::any_of(
                    compilers.begin(), compilers.end(),
                    [&executable](const CompilerOption& option) {
                        return option.executable == executable;
                    }
                );
                if (!alreadyFound) {
                    compilers.push_back({
                        compiler.first[0] == L'g' ? "MSYS2 GCC (C++20)" : "MSYS2 Clang (C++20)",
                        executable,
                        compiler.second
                    });
                }
            }
        }

        const char* localAppData = std::getenv("LOCALAPPDATA");
        if (localAppData != nullptr) {
            const std::filesystem::path pythonRoot =
                std::filesystem::path(localAppData) / "Programs" / "Python";
            std::error_code error;
            if (std::filesystem::is_directory(pythonRoot, error)) {
                std::vector<std::filesystem::path> pythonExecutables;
                for (std::filesystem::directory_iterator it(pythonRoot, error), end;
                     !error && it != end; it.increment(error)) {
                    if (it->is_directory(error)) {
                        const auto executable = it->path() / "python.exe";
                        if (std::filesystem::is_regular_file(executable, error)) {
                            pythonExecutables.push_back(executable);
                        }
                    }
                }
                std::sort(pythonExecutables.begin(), pythonExecutables.end());
                for (const auto& executable : pythonExecutables) {
                    std::string version = executable.parent_path().filename().string();
                    if (version.rfind("Python", 0) == 0) {
                        const std::string digits = version.substr(6);
                        version = digits.size() > 1
                            ? "Python " + digits.substr(0, 1) + "." + digits.substr(1)
                            : "Python";
                    }
                    compilers.push_back({
                        std::move(version),
                        executable,
                        CompilerKind::Python
                    });
                }
            }
        }

        const bool pythonFound = std::any_of(
            compilers.begin(), compilers.end(),
            [](const CompilerOption& option) { return option.kind == CompilerKind::Python; }
        );
        if (!pythonFound) {
            std::array<wchar_t, MAX_PATH> pythonPath{};
            if (SearchPathW(nullptr, L"python.exe", nullptr, static_cast<DWORD>(pythonPath.size()),
                            pythonPath.data(), nullptr) > 0) {
                compilers.push_back({"Python", pythonPath.data(), CompilerKind::Python});
            }
        }
#endif
        return compilers;
    }

    std::filesystem::path GetExecutableDirectory()
    {
#ifdef _WIN32
        std::vector<wchar_t> pathBuffer(512);
        while (pathBuffer.size() <= 32768) {
            const DWORD pathLength = GetModuleFileNameW(
                nullptr, pathBuffer.data(), static_cast<DWORD>(pathBuffer.size())
            );
            if (pathLength == 0) {
                return {};
            }
            if (pathLength < pathBuffer.size()) {
                return std::filesystem::path(
                    std::wstring(pathBuffer.data(), pathLength)
                ).parent_path();
            }
            pathBuffer.resize(pathBuffer.size() * 2);
        }
        return {};
#else
        return {};
#endif
    }

    ImFont* LoadInterfaceFont(ImGuiIO& io)
    {
        const std::filesystem::path fontPath =
            GetExecutableDirectory() / "assets" / "NotoSans-Regular.ttf";
        std::error_code error;
        if (!std::filesystem::is_regular_file(fontPath, error)) {
            std::cerr << "Bundled UI font was not found: " << fontPath.string() << '\n';
            return nullptr;
        }

        if (ImFont* font = io.Fonts->AddFontFromFileTTF(
                fontPath.string().c_str(), 17.0f, nullptr,
                io.Fonts->GetGlyphRangesCyrillic())) {
            return font;
        }
        std::cerr << "Could not load bundled UI font: " << fontPath.string() << '\n';
        return nullptr;
    }

#ifdef _WIN32
    std::wstring QuoteArgument(const std::wstring& argument)
    {
        std::wstring quoted = L"\"";
        std::size_t backslashes = 0;
        for (const wchar_t character : argument) {
            if (character == L'\\') {
                ++backslashes;
            } else if (character == L'"') {
                quoted.append(backslashes * 2 + 1, L'\\');
                quoted.push_back(L'"');
                backslashes = 0;
            } else {
                quoted.append(backslashes, L'\\');
                backslashes = 0;
                quoted.push_back(character);
            }
        }
        quoted.append(backslashes * 2, L'\\');
        quoted.push_back(L'"');
        return quoted;
    }

    std::vector<wchar_t> BuildChildEnvironment(
        const std::filesystem::path& executable,
        const std::filesystem::path& additionalSearchDirectory)
    {
        std::vector<std::wstring> entries;
        std::wstring currentPath;
        LPWCH environment = GetEnvironmentStringsW();
        if (environment != nullptr) {
            for (const wchar_t* entry = environment; *entry != L'\0';
                 entry += std::wcslen(entry) + 1) {
                const std::wstring value(entry);
                const std::size_t equal = value.find(L'=', value.front() == L'=' ? 1 : 0);
                if (equal == std::wstring::npos) {
                    continue;
                }
                const std::wstring name = value.substr(0, equal);
                if (CompareStringOrdinal(name.c_str(), static_cast<int>(name.size()),
                                         L"PATH", 4, TRUE) == CSTR_EQUAL) {
                    currentPath = value.substr(equal + 1);
                } else if (CompareStringOrdinal(name.c_str(), static_cast<int>(name.size()),
                                                L"PYTHONUTF8", 10, TRUE) != CSTR_EQUAL) {
                    entries.push_back(value);
                }
            }
            FreeEnvironmentStringsW(environment);
        }

        std::wstring childPath = executable.parent_path().wstring();
        if (!additionalSearchDirectory.empty() &&
            additionalSearchDirectory != executable.parent_path()) {
            childPath += L";" + additionalSearchDirectory.wstring();
        }
        if (!currentPath.empty()) {
            childPath += L";" + currentPath;
        }
        entries.push_back(L"PATH=" + childPath);
        entries.push_back(L"PYTHONUTF8=1");
        std::sort(entries.begin(), entries.end(), [](const std::wstring& left, const std::wstring& right) {
            const std::size_t leftEqual = left.find(L'=');
            const std::size_t rightEqual = right.find(L'=');
            const int comparison = CompareStringOrdinal(
                left.c_str(), static_cast<int>(leftEqual),
                right.c_str(), static_cast<int>(rightEqual),
                TRUE
            );
            return comparison == CSTR_LESS_THAN;
        });

        std::vector<wchar_t> block;
        for (const std::wstring& entry : entries) {
            block.insert(block.end(), entry.begin(), entry.end());
            block.push_back(L'\0');
        }
        block.push_back(L'\0');
        return block;
    }

    ExecutionResult RunProcess(
        const std::filesystem::path& executable,
        const std::vector<std::wstring>& arguments,
        const std::filesystem::path& workingDirectory,
        int timeoutMs,
        const std::filesystem::path& additionalSearchDirectory = {})
    {
        ExecutionResult result;
        std::wstring commandLine = QuoteArgument(executable.wstring());
        for (const std::wstring& argument : arguments) {
            commandLine.push_back(L' ');
            commandLine += QuoteArgument(argument);
        }
        std::vector<wchar_t> mutableCommandLine(commandLine.begin(), commandLine.end());
        mutableCommandLine.push_back(L'\0');

        SECURITY_ATTRIBUTES securityAttributes{};
        securityAttributes.nLength = sizeof(securityAttributes);
        securityAttributes.bInheritHandle = TRUE;
        HANDLE pipeRead = nullptr;
        HANDLE pipeWrite = nullptr;
        if (!CreatePipe(&pipeRead, &pipeWrite, &securityAttributes, 0)) {
            result.couldNotStart = true;
            result.output = "Could not create a process output pipe.";
            return result;
        }
        SetHandleInformation(pipeRead, HANDLE_FLAG_INHERIT, 0);

        HANDLE nullInput = CreateFileW(
            L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
            &securityAttributes, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr
        );
        if (nullInput == INVALID_HANDLE_VALUE) {
            CloseHandle(pipeRead);
            CloseHandle(pipeWrite);
            result.couldNotStart = true;
            result.output = "Could not open NUL as process input.";
            return result;
        }

        STARTUPINFOW startupInfo{};
        startupInfo.cb = sizeof(startupInfo);
        startupInfo.dwFlags = STARTF_USESTDHANDLES;
        startupInfo.hStdInput = nullInput;
        startupInfo.hStdOutput = pipeWrite;
        startupInfo.hStdError = pipeWrite;
        PROCESS_INFORMATION processInfo{};
        const std::wstring executablePath = executable.wstring();
        const std::wstring workingDirectoryPath = workingDirectory.wstring();
        std::vector<wchar_t> childEnvironment =
            BuildChildEnvironment(executable, additionalSearchDirectory);
        const BOOL created = CreateProcessW(
            executablePath.c_str(),
            mutableCommandLine.data(),
            nullptr,
            nullptr,
            TRUE,
            CREATE_NO_WINDOW | CREATE_SUSPENDED | CREATE_UNICODE_ENVIRONMENT,
            childEnvironment.data(),
            workingDirectoryPath.c_str(),
            &startupInfo,
            &processInfo
        );
        CloseHandle(nullInput);
        CloseHandle(pipeWrite);
        if (!created) {
            CloseHandle(pipeRead);
            result.couldNotStart = true;
            result.output = "CreateProcess failed with Windows error " +
                            std::to_string(GetLastError()) + ".";
            return result;
        }

        HANDLE job = CreateJobObjectW(nullptr, nullptr);
        if (job == nullptr) {
            TerminateProcess(processInfo.hProcess, 1);
            WaitForSingleObject(processInfo.hProcess, INFINITE);
            CloseHandle(processInfo.hThread);
            CloseHandle(processInfo.hProcess);
            CloseHandle(pipeRead);
            result.couldNotStart = true;
            result.output = "Could not create a process job.";
            return result;
        }

        JOBOBJECT_EXTENDED_LIMIT_INFORMATION jobLimits{};
        jobLimits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        const BOOL jobConfigured = SetInformationJobObject(
            job, JobObjectExtendedLimitInformation, &jobLimits, sizeof(jobLimits)
        );
        const BOOL jobAssigned = jobConfigured &&
            AssignProcessToJobObject(job, processInfo.hProcess);
        const DWORD resumeResult = jobAssigned ? ResumeThread(processInfo.hThread) : 0;
        if (!jobAssigned || resumeResult == static_cast<DWORD>(-1)) {
            TerminateProcess(processInfo.hProcess, 1);
            TerminateJobObject(job, 1);
            WaitForSingleObject(processInfo.hProcess, INFINITE);
            CloseHandle(job);
            CloseHandle(processInfo.hThread);
            CloseHandle(processInfo.hProcess);
            CloseHandle(pipeRead);
            result.couldNotStart = true;
            result.output = "Could not start the process inside its job.";
            return result;
        }
        CloseHandle(processInfo.hThread);

        const auto startTime = std::chrono::steady_clock::now();
        bool processFinished = false;
        bool outputTruncated = false;
        std::array<char, 4096> buffer{};
        while (!processFinished) {
            DWORD availableBytes = 0;
            if (PeekNamedPipe(pipeRead, nullptr, 0, nullptr, &availableBytes, nullptr) &&
                availableBytes > 0) {
                const DWORD bytesToRead = std::min<DWORD>(availableBytes, buffer.size());
                DWORD bytesRead = 0;
                if (ReadFile(pipeRead, buffer.data(), bytesToRead, &bytesRead, nullptr) && bytesRead > 0) {
                    const std::size_t remaining = MaxOutputBytes > result.output.size()
                        ? MaxOutputBytes - result.output.size()
                        : 0;
                    const std::size_t bytesToKeep = std::min<std::size_t>(remaining, bytesRead);
                    result.output.append(buffer.data(), bytesToKeep);
                    outputTruncated = outputTruncated || bytesToKeep < bytesRead;
                }
            }

            const DWORD waitResult = WaitForSingleObject(processInfo.hProcess, 20);
            if (waitResult == WAIT_OBJECT_0) {
                processFinished = true;
            } else if (waitResult == WAIT_FAILED) {
                TerminateJobObject(job, 1);
                result.couldNotStart = true;
                processFinished = true;
            }

            const auto elapsed = std::chrono::steady_clock::now() - startTime;
            if (!processFinished && elapsed >= std::chrono::milliseconds(timeoutMs)) {
                result.timedOut = true;
                TerminateJobObject(job, 1);
                WaitForSingleObject(processInfo.hProcess, 2000);
                processFinished = true;
            }
        }

        DWORD exitCode = 1;
        if (!result.timedOut && !result.couldNotStart &&
            GetExitCodeProcess(processInfo.hProcess, &exitCode)) {
            result.exitCode = static_cast<int>(exitCode);
        }
        DWORD remainingBytes = 0;
        while (PeekNamedPipe(pipeRead, nullptr, 0, nullptr, &remainingBytes, nullptr) &&
               remainingBytes > 0) {
            DWORD bytesRead = 0;
            const DWORD bytesToRead = std::min<DWORD>(remainingBytes, buffer.size());
            if (!ReadFile(pipeRead, buffer.data(), bytesToRead, &bytesRead, nullptr) || bytesRead == 0) {
                break;
            }
            const std::size_t remaining = MaxOutputBytes > result.output.size()
                ? MaxOutputBytes - result.output.size()
                : 0;
            const std::size_t bytesToKeep = std::min<std::size_t>(remaining, bytesRead);
            result.output.append(buffer.data(), bytesToKeep);
            outputTruncated = outputTruncated || bytesToKeep < bytesRead;
        }
        result.outputTruncated = outputTruncated;
        CloseHandle(pipeRead);
        CloseHandle(processInfo.hProcess);
        CloseHandle(job);
        return result;
    }
#endif

    ExecutionResult ExecuteSubmission(
        const CompilerOption& compiler,
        const std::string& code,
        int submissionNumber)
    {
        ExecutionResult result;
#ifndef _WIN32
        (void)compiler;
        (void)code;
        (void)submissionNumber;
        result.couldNotStart = true;
        result.output = "Code execution is currently supported on Windows only.";
        return result;
#else
        std::filesystem::path workDirectory;
        try {
            workDirectory = std::filesystem::temp_directory_path() / "CodeHub" /
                (std::to_string(GetCurrentProcessId()) + "_" +
                 std::to_string(submissionNumber) + "_" +
                 std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
            if (!std::filesystem::create_directories(workDirectory)) {
                result.couldNotStart = true;
                result.output = "The temporary source directory already exists.";
                workDirectory.clear();
                return result;
            }

            const bool python = compiler.kind == CompilerKind::Python;
            const std::filesystem::path sourcePath = workDirectory /
                (python ? "main.py" : "main.cpp");
            std::ofstream sourceFile(sourcePath, std::ios::binary);
            if (!sourceFile) {
                result.couldNotStart = true;
                result.output = "Could not create the source file.";
                std::error_code cleanupError;
                std::filesystem::remove_all(workDirectory, cleanupError);
                return result;
            }
            sourceFile.write(code.data(), static_cast<std::streamsize>(code.size()));
            sourceFile.close();
            if (!sourceFile) {
                result.couldNotStart = true;
                result.output = "Could not write the source file.";
                std::error_code cleanupError;
                std::filesystem::remove_all(workDirectory, cleanupError);
                return result;
            }

            if (python) {
                result = RunProcess(
                    compiler.executable, {sourcePath.wstring()}, workDirectory, RunTimeoutMs
                );
            } else {
                const std::filesystem::path executablePath = workDirectory / "program.exe";
                ExecutionResult compileResult = RunProcess(
                    compiler.executable,
                    {L"-std=c++20", sourcePath.wstring(), L"-o", executablePath.wstring()},
                    workDirectory,
                    CompileTimeoutMs
                );
                if (compileResult.couldNotStart || compileResult.timedOut || compileResult.exitCode != 0) {
                    result = std::move(compileResult);
                } else {
                    result = RunProcess(
                        executablePath, {}, workDirectory, RunTimeoutMs,
                        compiler.executable.parent_path()
                    );
                    if (!compileResult.output.empty()) {
                        const std::size_t remaining = MaxOutputBytes > compileResult.output.size()
                            ? MaxOutputBytes - compileResult.output.size()
                            : 0;
                        const std::size_t runOutputSize = result.output.size();
                        const std::size_t bytesToAppend = std::min(remaining, result.output.size());
                        compileResult.output.append(result.output.data(), bytesToAppend);
                        result.output = std::move(compileResult.output);
                        result.outputTruncated = result.outputTruncated ||
                            bytesToAppend < runOutputSize;
                    }
                    result.outputTruncated = result.outputTruncated || compileResult.outputTruncated;
                }
            }
        } catch (const std::filesystem::filesystem_error& error) {
            result.couldNotStart = true;
            result.output = std::string("File system error: ") + error.what();
        } catch (const std::exception& error) {
            result.couldNotStart = true;
            result.output = std::string("Execution error: ") + error.what();
        }

        if (!workDirectory.empty()) {
            std::error_code cleanupError;
            std::filesystem::remove_all(workDirectory, cleanupError);
            if (cleanupError) {
                std::cerr << "Could not remove temporary source directory: "
                          << cleanupError.message() << '\n';
            }
        }
        return result;
#endif
    }

    TextKey GetExecutionStatus(const ExecutionResult& result)
    {
        if (result.timedOut) {
            return TextKey::TimedOut;
        }
        if (result.couldNotStart) {
            return TextKey::CompilerLaunchFailed;
        }
        return result.exitCode == 0 ? TextKey::Success : TextKey::Failed;
    }

    void CollectExecutionResult(AppState& state)
    {
        if (!state.executionPending ||
            state.pendingExecution.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
            return;
        }

        ExecutionResult result;
        try {
            result = state.pendingExecution.get();
        } catch (const std::exception& error) {
            state.executionPending = false;
            state.lastStatus = TextKey::CompilerLaunchFailed;
            state.lastOutput = error.what();
            return;
        }
        state.executionPending = false;
        state.lastStatus = GetExecutionStatus(result);
        state.lastOutput = result.output;
        state.submissions.push_back({
            state.nextSubmissionNumber++,
            state.lastCompiler,
            state.lastCode,
            std::move(result)
        });
    }

    void DrawHeader(AppState& state, ImDrawList* draw, const ImVec2 rootPos, const ImVec2 rootSize)
    {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(17, 12, 27, 255));
        ImGui::BeginChild("Header", ImVec2(0.0f, HeaderHeight), ImGuiChildFlags_None);
        const float headerHeight = ImGui::GetWindowHeight();
        ImGui::SetCursorPos(ImVec2(24.0f, (headerHeight - ImGui::GetTextLineHeight()) * 0.5f));
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(238, 224, 255, 255));
        ImGui::TextUnformatted("CodeHub");
        ImGui::PopStyleColor();

        constexpr float codeButtonWidth = 112.0f;
        constexpr float historyButtonWidth = 142.0f;
        constexpr float buttonGap = 10.0f;
        constexpr float rightPadding = 24.0f;
        const float buttonY = (headerHeight - ImGui::GetFrameHeight()) * 0.5f;
        const float historyX = ImGui::GetWindowWidth() - rightPadding - historyButtonWidth;
        const float codeX = historyX - buttonGap - codeButtonWidth;

        ImGui::SetCursorPos(ImVec2(112.0f, buttonY));
        if (ImGui::BeginCombo(
                "##language",
                Translate(state.language, state.language == Language::English ? TextKey::English : TextKey::Kazakh),
                ImGuiComboFlags_NoArrowButton)) {
            for (const Language language : {Language::English, Language::Kazakh}) {
                const TextKey languageLabel =
                    language == Language::English ? TextKey::English : TextKey::Kazakh;
                if (ImGui::Selectable(Translate(state.language, languageLabel), state.language == language)) {
                    state.language = language;
                }
            }
            ImGui::EndCombo();
        }

        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(52, 31, 78, 255));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(107, 55, 157, 255));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(143, 75, 204, 255));
        ImGui::SetCursorPos(ImVec2(codeX, buttonY));
        if (ImGui::Button(Translate(state.language, TextKey::Code), ImVec2(codeButtonWidth, 0.0f))) {
            state.codePanelOpen = true;
        }
        ImGui::SetCursorPos(ImVec2(historyX, buttonY));
        if (ImGui::Button(Translate(state.language, TextKey::History), ImVec2(historyButtonWidth, 0.0f))) {
            state.drawerTab = 1;
            state.drawerOpen = true;
        }
        ImGui::PopStyleColor(3);

        draw->AddLine(
            ImVec2(rootPos.x, rootPos.y + headerHeight - 1.0f),
            ImVec2(rootPos.x + rootSize.x, rootPos.y + headerHeight - 1.0f),
            IM_COL32(142, 65, 214, 220),
            1.0f
        );
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    void DrawHomeContent(AppState& state)
    {
        ImGui::SetCursorPos(ImVec2(28.0f, 24.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(233, 220, 248, 255));
        ImGui::TextUnformatted(Translate(state.language, TextKey::Home));
        ImGui::PopStyleColor();

        const float contentWidth = ImGui::GetContentRegionAvail().x - 56.0f;
        const float cardWidth = std::max(120.0f, (contentWidth - 20.0f) / 3.0f);
        ImGui::SetCursorPos(ImVec2(28.0f, 70.0f));
        const char* cardNames[] = {
            Translate(state.language, TextKey::Compiler),
            Translate(state.language, TextKey::Submissions),
            Translate(state.language, TextKey::Status)
        };
        for (int i = 0; i < 3; ++i) {
            if (i > 0) {
                ImGui::SameLine(0.0f, 10.0f);
            }
            ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(22, 16, 33, 255));
            ImGui::BeginChild(cardNames[i], ImVec2(cardWidth, 90.0f), ImGuiChildFlags_Borders);
            ImGui::SetCursorPos(ImVec2(14.0f, 13.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(155, 140, 171, 255));
            ImGui::TextUnformatted(cardNames[i]);
            ImGui::PopStyleColor();
            ImGui::SetCursorPos(ImVec2(14.0f, 43.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(225, 198, 255, 255));
            if (i == 0) {
                if (state.compilers.empty()) {
                    ImGui::TextUnformatted("-");
                } else {
                    ImGui::TextUnformatted(state.compilers[state.selectedCompiler].name.c_str());
                }
            } else if (i == 1) {
                ImGui::Text("%zu", state.submissions.size());
            } else {
                ImGui::TextWrapped("%s", Translate(state.language, state.lastStatus));
            }
            ImGui::PopStyleColor();
            ImGui::EndChild();
            ImGui::PopStyleColor();
        }

        ImGui::SetCursorPos(ImVec2(28.0f, 190.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(196, 139, 255, 255));
        ImGui::TextUnformatted(Translate(state.language, TextKey::Workspace));
        ImGui::PopStyleColor();
        ImGui::SetCursorPos(ImVec2(28.0f, 222.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(143, 132, 157, 255));
        ImGui::TextWrapped("%s", Translate(state.language, TextKey::HomeHint));
        ImGui::PopStyleColor();

        ImGui::SetCursorPos(ImVec2(28.0f, 292.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(83, 43, 119, 255));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(116, 61, 162, 255));
        if (ImGui::Button(Translate(state.language, TextKey::OpenEditor), ImVec2(210.0f, 38.0f))) {
            state.codePanelOpen = true;
        }
        ImGui::PopStyleColor(2);
    }

    void DrawRootWindow(AppState& state)
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(8, 7, 13, 255));

        constexpr ImGuiWindowFlags rootFlags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::Begin("CodeHubRoot", nullptr, rootFlags);
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const ImVec2 rootPos = ImGui::GetWindowPos();
        const ImVec2 rootSize = ImGui::GetWindowSize();
        DrawHeader(state, draw, rootPos, rootSize);

        ImGui::SetCursorPosY(HeaderHeight + PanelGap);
        const float bodyHeight = std::max(
            0.0f,
            ImGui::GetContentRegionAvail().y - FooterHeight - PanelGap
        );
        ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(12, 10, 19, 255));
        ImGui::BeginChild("WorkspacePanels", ImVec2(0.0f, bodyHeight), ImGuiChildFlags_None);

        const float availableWidth = ImGui::GetContentRegionAvail().x;
        const float sidebarWidth = std::min(SidebarWidth, availableWidth * 0.32f);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(20, 15, 30, 255));
        ImGui::BeginChild("TasksSidebar", ImVec2(sidebarWidth, 0.0f), ImGuiChildFlags_Borders);
        ImGui::SetCursorPos(ImVec2(18.0f, 20.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(196, 139, 255, 255));
        ImGui::TextUnformatted(Translate(state.language, TextKey::Tasks));
        ImGui::PopStyleColor();
        ImGui::SetCursorPos(ImVec2(18.0f, 54.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(143, 132, 157, 255));
        ImGui::TextWrapped("%s", Translate(state.language, TextKey::TasksEmpty));
        ImGui::PopStyleColor();
        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::SameLine(0.0f, PanelGap);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(10, 9, 16, 255));
        ImGui::BeginChild("MainWorkspace", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders);
        const ImVec2 workspacePos = ImGui::GetWindowPos();
        const ImVec2 workspaceSize = ImGui::GetWindowSize();
        draw->AddRect(
            workspacePos,
            ImVec2(workspacePos.x + workspaceSize.x, workspacePos.y + workspaceSize.y),
            IM_COL32(89, 46, 128, 180),
            8.0f
        );
        DrawHomeContent(state);
        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::SetCursorPosY(rootSize.y - FooterHeight);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(17, 12, 27, 255));
        ImGui::BeginChild("StatusBar", ImVec2(0.0f, FooterHeight), ImGuiChildFlags_None);
        ImGui::SetCursorPos(ImVec2(18.0f, (FooterHeight - ImGui::GetTextLineHeight()) * 0.5f));
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(177, 130, 222, 255));
        ImGui::Text("%s: %s (%zu)",
                    Translate(state.language, TextKey::Status),
                    Translate(state.language, state.executionPending ? TextKey::Running : state.lastStatus),
                    state.submissions.size());
        ImGui::PopStyleColor();
        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }

    void DrawCodePanel(AppState& state)
    {
        if (!state.codePanelOpen) {
            return;
        }

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const float panelWidth = std::min(780.0f, std::max(360.0f, viewport->WorkSize.x - 32.0f));
        const float panelHeight = std::min(560.0f, std::max(360.0f, viewport->WorkSize.y - 32.0f));
        ImGui::SetNextWindowPos(
            ImVec2(viewport->WorkPos.x + (viewport->WorkSize.x - panelWidth) * 0.5f,
                   viewport->WorkPos.y + (viewport->WorkSize.y - panelHeight) * 0.5f),
            ImGuiCond_Always
        );
        ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(18, 13, 27, 255));
        ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(126, 67, 177, 255));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f, 18.0f));

        constexpr ImGuiWindowFlags panelFlags =
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoSavedSettings;
        ImGui::Begin(Translate(state.language, TextKey::EditorTitle), nullptr, panelFlags);
        constexpr float closeWidth = 72.0f;
        ImGui::SetCursorPos(ImVec2(20.0f, 18.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(233, 220, 248, 255));
        ImGui::TextUnformatted(Translate(state.language, TextKey::SubmitSolution));
        ImGui::PopStyleColor();
        ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - 20.0f - closeWidth, 14.0f));
        if (ImGui::Button(Translate(state.language, TextKey::Close), ImVec2(closeWidth, 0.0f))) {
            state.codePanelOpen = false;
        }

        ImGui::SetCursorPos(ImVec2(20.0f, 58.0f));
        ImGui::Spacing();
        ImGui::TextUnformatted(Translate(state.language, TextKey::ChooseCompiler));
        if (state.compilers.empty()) {
            ImGui::TextDisabled("%s", Translate(state.language, TextKey::NoCompilers));
        } else {
            state.selectedCompiler = std::clamp(
                state.selectedCompiler, 0, static_cast<int>(state.compilers.size()) - 1
            );
            ImGui::SetNextItemWidth(-1.0f);
            if (ImGui::BeginCombo(
                    "##compiler", state.compilers[state.selectedCompiler].name.c_str())) {
                for (int i = 0; i < static_cast<int>(state.compilers.size()); ++i) {
                    const bool selected = state.selectedCompiler == i;
                    if (ImGui::Selectable(state.compilers[i].name.c_str(), selected)) {
                        state.selectedCompiler = i;
                    }
                    if (selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
        }

        ImGui::Spacing();
        ImGui::TextUnformatted(Translate(state.language, TextKey::YourCode));
        const float editorHeight = std::max(120.0f, ImGui::GetContentRegionAvail().y - 132.0f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(9, 8, 14, 255));
        ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(80, 49, 106, 255));
        ImGui::InputTextMultiline(
            "##source_code",
            state.codeBuffer.data(),
            state.codeBuffer.size(),
            ImVec2(-1.0f, editorHeight),
            ImGuiInputTextFlags_AllowTabInput
        );
        ImGui::PopStyleColor(2);

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(151, 137, 166, 255));
        ImGui::TextWrapped("%s", Translate(state.language, TextKey::TabHint));
        ImGui::TextWrapped("%s", Translate(state.language, TextKey::CompilationPending));
        ImGui::PopStyleColor();
        constexpr float sendWidth = 120.0f;
        const float sendX = ImGui::GetWindowWidth() - 20.0f - sendWidth;
        ImGui::SetCursorPosX(sendX);
        ImGui::BeginDisabled(state.executionPending || state.compilers.empty());
        if (ImGui::Button(
                Translate(state.language, state.executionPending ? TextKey::Running : TextKey::Send),
                ImVec2(sendWidth, 34.0f))) {
            if (state.codeBuffer[0] == '\0') {
                state.lastStatus = TextKey::EnterCode;
                state.lastCompiler.clear();
                state.lastCode.clear();
                state.lastOutput.clear();
            } else {
                const CompilerOption compiler = state.compilers[state.selectedCompiler];
                state.lastCompiler = compiler.name;
                state.lastCode = state.codeBuffer.data();
                state.lastStatus = TextKey::Running;
                state.lastOutput.clear();
                try {
                    state.pendingExecution = std::async(
                        std::launch::async,
                        ExecuteSubmission,
                        compiler,
                        state.lastCode,
                        state.nextSubmissionNumber
                    );
                    state.executionPending = true;
                } catch (const std::exception& error) {
                    state.lastStatus = TextKey::CompilerLaunchFailed;
                    state.lastOutput = error.what();
                }
            }
        }
        ImGui::EndDisabled();

        ImGui::End();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
    }

    void DrawHistoryDrawer(AppState& state)
    {
        if (!state.drawerOpen) {
            state.drawerOffset = 0.0f;
            return;
        }

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const float drawerWidth = std::min(430.0f, std::max(300.0f, viewport->WorkSize.x - 24.0f));
        const float drawerHeight = viewport->WorkSize.y - 20.0f;
        const float drawerX = viewport->WorkPos.x + viewport->WorkSize.x - drawerWidth + state.drawerOffset;
        ImGui::SetNextWindowPos(ImVec2(drawerX, viewport->WorkPos.y + 10.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(drawerWidth, drawerHeight), ImGuiCond_Always);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(18, 13, 27, 255));
        ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(126, 67, 177, 255));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 14.0f));

        constexpr ImGuiWindowFlags drawerFlags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoSavedSettings;
        ImGui::Begin("Status and History", nullptr, drawerFlags);

        ImGui::SetCursorPos(ImVec2(12.0f, 10.0f));
        const ImVec2 handlePos = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##drawer_drag_handle", ImVec2(drawerWidth - 72.0f, 34.0f));
        ImGui::GetWindowDrawList()->AddText(
            ImVec2(handlePos.x + 8.0f, handlePos.y + 9.0f),
            IM_COL32(208, 174, 237, 255),
            Translate(state.language, TextKey::DragToClose)
        );
        if (ImGui::IsItemActive()) {
            state.drawerOffset = std::clamp(
                state.drawerOffset + ImGui::GetIO().MouseDelta.x,
                0.0f,
                drawerWidth
            );
            if (state.drawerOffset >= drawerWidth * 0.65f) {
                state.drawerOpen = false;
            }
        } else if (state.drawerOffset > 0.0f) {
            state.drawerOffset = 0.0f;
        }

        ImGui::SetCursorPos(ImVec2(drawerWidth - 48.0f, 10.0f));
        if (ImGui::Button("X", ImVec2(32.0f, 30.0f))) {
            state.drawerOpen = false;
        }

        ImGui::SetCursorPos(ImVec2(12.0f, 54.0f));
        constexpr float tabGap = 8.0f;
        const float tabWidth = (drawerWidth - 40.0f - tabGap) * 0.5f;
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(52, 31, 78, 255));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(107, 55, 157, 255));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(143, 75, 204, 255));
        if (ImGui::Button(
                Translate(state.language, TextKey::CurrentCode),
                ImVec2(tabWidth, 36.0f))) {
            state.drawerTab = 0;
        }
        ImGui::SameLine(0.0f, tabGap);
        if (ImGui::Button(
                Translate(state.language, TextKey::SubmissionHistory),
                ImVec2(tabWidth, 36.0f))) {
            state.drawerTab = 1;
        }
        ImGui::PopStyleColor(3);

        ImGui::SetCursorPos(ImVec2(12.0f, 104.0f));
        ImGui::BeginChild("DrawerContent", ImVec2(-1.0f, -1.0f), ImGuiChildFlags_Borders);
        if (state.drawerTab == 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(196, 139, 255, 255));
            ImGui::TextUnformatted(Translate(state.language, TextKey::CurrentCodeStatus));
            ImGui::PopStyleColor();
            ImGui::Separator();
            ImGui::TextWrapped("%s", Translate(state.language, state.lastStatus));
            if (!state.lastCompiler.empty()) {
                ImGui::Spacing();
                ImGui::Text("%s: %s", Translate(state.language, TextKey::Compiler),
                            state.lastCompiler.c_str());
            }
            if (state.lastCode.empty()) {
                ImGui::Spacing();
                ImGui::TextDisabled("%s", Translate(state.language, TextKey::NoSubmissions));
            } else {
                ImGui::Spacing();
                ImGui::TextUnformatted(Translate(state.language, TextKey::LatestCode));
                const float sourceHeight = std::max(80.0f, ImGui::GetContentRegionAvail().y * 0.48f);
                ImGui::BeginChild("CurrentCodePreview", ImVec2(0.0f, sourceHeight), ImGuiChildFlags_Borders);
                ImGui::TextUnformatted(state.lastCode.c_str());
                ImGui::EndChild();
                ImGui::TextUnformatted(Translate(state.language, TextKey::Output));
                ImGui::BeginChild("CurrentOutput", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders);
                if (state.lastOutput.empty()) {
                    ImGui::TextDisabled("%s", Translate(state.language, TextKey::NoOutput));
                } else {
                    ImGui::TextUnformatted(state.lastOutput.c_str());
                }
                if (!state.submissions.empty() && state.submissions.back().result.outputTruncated) {
                    ImGui::TextDisabled("%s", Translate(state.language, TextKey::OutputTruncated));
                }
                ImGui::EndChild();
            }
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(196, 139, 255, 255));
            ImGui::TextUnformatted(Translate(state.language, TextKey::SubmissionHistory));
            ImGui::PopStyleColor();
            ImGui::Separator();
            if (state.submissions.empty()) {
                ImGui::TextDisabled("%s", Translate(state.language, TextKey::HistoryEmpty));
            } else {
                for (auto submission = state.submissions.rbegin();
                     submission != state.submissions.rend(); ++submission) {
                    ImGui::PushID(submission->number);
                    ImGui::Text(
                        "%s #%d | %s",
                        state.language == Language::Kazakh ? "Жіберілім" : "Submission",
                        submission->number,
                        submission->compiler.c_str()
                    );
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(151, 137, 166, 255));
                    ImGui::TextWrapped("%s", submission->code.c_str());
                    ImGui::PopStyleColor();
                    ImGui::TextWrapped("%s", Translate(state.language, GetExecutionStatus(submission->result)));
                    ImGui::Separator();
                    ImGui::PopID();
                }
            }
        }
        ImGui::EndChild();

        ImGui::End();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
    }

    void DrawInterface(AppState& state)
    {
        CollectExecutionResult(state);
        DrawRootWindow(state);
        DrawHistoryDrawer(state);
        DrawCodePanel(state);
    }
}

int main()
{
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(1080, 720, "CodeHub", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return 1;
    }

    glfwSetWindowSizeLimits(window, 900, 600, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();
    ImFont* font = LoadInterfaceFont(io);
    if (font == nullptr) {
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    io.FontDefault = font;
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowBorderSize = 1.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 7.0f;
    style.FrameBorderSize = 1.0f;
    style.ItemSpacing = ImVec2(8.0f, 8.0f);

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        std::cerr << "Failed to initialize ImGui GLFW backend\n";
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    if (!ImGui_ImplOpenGL3_Init()) {
        std::cerr << "Failed to initialize ImGui OpenGL backend\n";
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    AppState state;
    state.compilers = FindCompilers();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        DrawInterface(state);
        ImGui::Render();
        int displayWidth = 0;
        int displayHeight = 0;
        glfwGetFramebufferSize(window, &displayWidth, &displayHeight);
        glViewport(0, 0, displayWidth, displayHeight);
        glClearColor(0.035f, 0.025f, 0.055f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
