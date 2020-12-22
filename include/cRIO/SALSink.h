/*
 * Developed for the Vera C. Rubin Observatory Telescope & Site Software Systems.
 * This product includes software developed by the Vera C.Rubin Observatory Project
 * (https://www.lsst.org). See the COPYRIGHT file at the top-level directory of
 * this distribution for details of code ownership.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SALSINK_H_
#define SALSINK_H_

/**
 * Sink to send all M1M3 spdlog messages to SAL using logMessage event. Macro which creates template class. To
 * use it, just create mySALSink.h with the following content:
 *
 * @code{.cpp}
 *
 * #include <SALSink.h>
 *
 * SALSinkMacro(MTM1M3)
 *
 * @endcode
 *
 * @see https://github.com/gabime/spdlog/wiki/4.-Sinks
 */
#define SALSinkMacro(remoteName)                                                 \
#include "spdlog/sinks/base_sink.h";                                         \
#include <SAL_##remoteName.h>;                                               \
#include <memory>;                                                           \
    template <typename Mutex>                                                    \
    class SALSink : public spdlog::sinks::base_sink<Mutex> {                     \
    public:                                                                      \
        SALSink(std::shared_ptr<SAL_##remoteName> remote) {                      \
            _remote = remote;                                                    \
            _remote->salEventPub((char*)#remoteName "_logevent_logMessage");     \
        }                                                                        \
                                                                                 \
    protected:                                                                   \
        void sink_it_(const spdlog::details::log_msg& msg) override {            \
            spdlog::memory_buf_t formatted;                                      \
            spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted); \
                                                                                 \
            remoteName##_logevent_logMessageC message;                           \
            message.name = #remoteName;                                          \
            message.level = msg.level * 10;                                      \
            message.message = fmt::to_string(msg.payload);                       \
            message.traceback = "";                                              \
            message.filePath = msg.source.filename;                              \
            message.functionName = msg.source.funcname;                          \
            message.lineNumber = msg.source.line;                                \
            message.process = getpid();                                          \
                                                                                 \
            _remote->logEvent_logMessage(&message, 0);                           \
        }                                                                        \
                                                                                 \
        void flush_() override {}                                                \
                                                                                 \
    private:                                                                     \
        std::shared_ptr<SAL_##remoteName> _remote;                               \
    };                                                                           \
                                                                                 \
#include "spdlog/details/null_mutex.h";                                      \
#include <mutex>;                                                            \
    using SALSink_mt = SALSink<std::mutex>;                                      \
    using SALSink_st = SALSink<spdlog::details::null_mutex>;

#endif  // SALSINK_H_
