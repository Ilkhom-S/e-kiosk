import re
import os

# List of cpp files to update
cpp_files = [
    'src/modules/UpdateEngine/src/ReportBuilder.cpp',
    'src/modules/UpdateEngine/src/Updater.cpp',
    'src/modules/UpdateEngine/src/Component.cpp',
    'src/modules/UpdateEngine/src/File.cpp',
    'src/modules/UpdateEngine/src/Folder.cpp',
    'src/modules/UpdateEngine/src/Package.cpp',
    'src/modules/UpdateEngine/src/WindowsBITS.cpp',
    'src/modules/UpdateEngine/src/WindowsBITS_p.cpp'
]

# Member variable replacements
replacements = {
    'mWorkDirectory': 'm_WorkDirectory',
    'mReport': 'm_Report',
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

for file_path in cpp_files:
    if os.path.exists(file_path):
        with open(file_path, 'r') as f:
            content = f.read()

        for old, new in replacements.items():
            content = re.sub(r'\b' + old + r'\b', new, content)

        with open(file_path, 'w') as f:
            f.write(content)

        print(f'Updated {file_path}')

print('Updated all UpdateEngine cpp files')
