import re

# Update ReportBuilder.h
with open('include/UpdateEngine/ReportBuilder.h', 'r') as f:
    content = f.read()
content = re.sub(r'\bmWorkDirectory\b', 'm_WorkDirectory', content)
content = re.sub(r'\bmReport\b', 'm_Report', content)
with open('include/UpdateEngine/ReportBuilder.h', 'w') as f:
    f.write(content)

# Update Updater.h
with open('include/UpdateEngine/Updater.h', 'r') as f:
    content = f.read()
replacements = {
    'mConfigURL': 'm_ConfigURL',
    'mUpdateURL': 'm_UpdateURL',
    'mVersion': 'm_Version',
    'mAP': 'm_AP',
    'mAcceptedKeys': 'm_AcceptedKeys',
    'mAppId': 'm_AppId',
    'mConfiguration': 'm_Configuration',
    'mUpdateComponents': 'm_UpdateComponents',
    'mOptionalComponents': 'm_OptionalComponents',
    'mRequiredFiles': 'm_RequiredFiles',
    'mMD5': 'm_MD5',
    'mNetworkTaskManager': 'm_NetworkTaskManager',
    'mWorkingDir': 'm_WorkingDir',
    'mExceptionDirs': 'm_ExceptionDirs',
    'mComponentsContent': 'm_ComponentsContent',
    'mComponentsSignature': 'm_ComponentsSignature',
    'mComponentsRevision': 'm_ComponentsRevision',
    'mComponents': 'm_Components',
    'mActiveTasks': 'm_ActiveTasks'
}
for old, new in replacements.items():
    content = re.sub(r'\b' + old + r'\b', new, content)
with open('include/UpdateEngine/Updater.h', 'w') as f:
    f.write(content)

print('Renamed member variables in UpdateEngine headers')
