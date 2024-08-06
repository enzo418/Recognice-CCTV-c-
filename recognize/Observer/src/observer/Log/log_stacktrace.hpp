#pragma once

#define UNW_LOCAL_ONLY

typedef void (*obs_sig_hndl)(int);

void ObserverRegisterSignalHandlers(obs_sig_hndl);
void ObserverLogStackTrace(void);
void ObserverCallOldSignalHandlers(int);