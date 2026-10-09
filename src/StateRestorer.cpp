#include "StateRestorer.h"
#include "SystemControl.h"
#include "Logger.h"
void StateRestorer::CaptureState(DWORD pid) {
    m_originalPowerPlan = SystemControl::Get().GetActivePowerPlan();
    DEVMODEW dm; ZeroMemory(&dm, sizeof(dm)); dm.dmSize = sizeof(dm);
    if (EnumDisplaySettingsW(NULL, ENUM_CURRENT_SETTINGS, &dm)) {
        m_originalDisplaySettings = dm;
    }
}
void StateRestorer::RegisterModifiedProcess(DWORD pid, DWORD origPriority, DWORD_PTR origAffinity) {
    for (auto& p : m_modifiedProcesses) if (p.pid == pid) return;
    m_modifiedProcesses.push_back({pid, origPriority, origAffinity});
}
void StateRestorer::RestoreState() {
    if (m_originalPowerPlan.has_value()) {
        SystemControl::Get().SetActivePowerPlan(m_originalPowerPlan.value());
        m_originalPowerPlan.reset();
    }
    if (m_originalDisplaySettings.has_value()) {
        ChangeDisplaySettingsExW(NULL, &m_originalDisplaySettings.value(), NULL, CDS_FULLSCREEN, NULL);
        m_originalDisplaySettings.reset();
    }
    for (const auto& proc : m_modifiedProcesses) {
        SystemControl::Get().SetProcessPriority(proc.pid, proc.originalPriority);
        if (proc.originalAffinity != 0) {
            SystemControl::Get().SetProcessAffinity(proc.pid, proc.originalAffinity);
        }
    }
    m_modifiedProcesses.clear();
}
