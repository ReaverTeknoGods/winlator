"""Exercise actual bootstrap cache/callback bodies; fake HWND APIs, native Windows guard.

Run on Windows with MSVC installed. --source can test a focused source proposal;
--output retains generated source, compiler output and results. No device access.
The independent pre-change Git object remains fixed after future commits.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

BASELINE_COMMIT='c840e7a7095e4b12667d36a6caf7b1432559cf8f'
BASELINE_BLOB='d77a4eca41b4069f45e841c6850bb2ac9664ddd4'
RELATIVE='native/teknoparrot-bridge/windows_path_bootstrap.c'

def sha(raw):return hashlib.sha256(raw).hexdigest()
def body(source,signature):
    begin=source.index(signature);end=source.index('{',begin)+1;depth=1
    while depth:
        if source[end]=='{':depth+=1
        elif source[end]=='}':depth-=1
        end+=1
    return source[begin:end]

def command(args,output,log):
    value=subprocess.run(args,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    (output/log).write_bytes(value.stdout)
    if value.returncode:raise RuntimeError(str(args[0])+' failed; retained '+str(output/log))
    return value.stdout.decode('utf-8',errors='replace')

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root',type=Path,default=Path(__file__).resolve().parents[3])
    p.add_argument('--source',type=Path)
    p.add_argument('--output',type=Path)
    a=p.parse_args();root=a.root.resolve();source=a.source or root/RELATIVE
    output=a.output or Path(tempfile.mkdtemp(prefix='taisen-client-policy-'))
    output.mkdir(parents=True,exist_ok=True)
    assert not (output/'result.json').exists(),'fresh test output required'
    object_name=BASELINE_COMMIT+':'+RELATIVE
    got=subprocess.check_output(['git','-C',str(root),'rev-parse',object_name]).decode().strip()
    if got!=BASELINE_BLOB:raise RuntimeError('Independent pre-change Git blob differs')
    baseline=subprocess.check_output(['git','-C',str(root),'show',object_name]).decode('utf-8')
    raw=source.read_bytes();actual=raw.decode('utf-8').replace('\r\n','\n')
    prefix=actual[:actual.index('static BOOL environment_flag_enabled(')]
    signatures=['static BOOL process_list_contains(','static BOOL taisen_client_preservation_enabled(',
        'static BOOL taisen_window_identity(','static BOOL preserve_taisen_client(',
        'static BOOL CALLBACK apply_window_policy(']
    methods='\n\n'.join(body(actual,s)for s in signatures)
    old=body(baseline,'static BOOL CALLBACK apply_window_policy(').replace('apply_window_policy(','apply_baseline_window_policy(',1)
    template=Path(__file__).with_name('taisen-client-policy-fixture.c.in').read_text(encoding='utf-8')
    fixture=template.replace('ACTUAL_TYPES',prefix).replace('ACTUAL_METHODS',methods+'\n\n'+old)
    if 'wmain('in fixture or 'CreateProcess'in fixture:raise RuntimeError('Policy fixture must not contain bootstrap execution')
    (output/'fixture.c').write_text(fixture,encoding='utf-8')
    # Unlike the fake-Wine policy matrix, this uses real Windows exports and
    # the actual production flag function to prove opt-in cannot enable Windows.
    canary='#define UNICODE\n#define _UNICODE\n#include <windows.h>\n#include <wchar.h>\n#include <stdio.h>\n'+body(actual,'static BOOL taisen_client_preservation_enabled(')+r'''
int wmain(void) {
    const wchar_t* values[]={NULL,L"",L"1",L"01",L"true"};unsigned int i;
    if(GetProcAddress(GetModuleHandleW(L"ntdll.dll"),"wine_get_version")!=NULL)return 90;
    for(i=0;i<ARRAYSIZE(values);++i) {
        if(!SetEnvironmentVariableW(L"TP_ANDROID_TAISEN_PRESERVE_CLIENT",values[i]))return 91;
        if(taisen_client_preservation_enabled())return 92;
    }
    wprintf(L"PASS 5 actual native-Windows optin guards; correction disabled\n");return 0;
}
'''
    (output/'windows-canary.c').write_text(canary,encoding='utf-8')
    vswhere=Path(os.environ.get('ProgramFiles(x86)','C:/Program Files (x86)'))/'Microsoft Visual Studio/Installer/vswhere.exe'
    install=subprocess.check_output([str(vswhere),'-latest','-products','*','-requires','Microsoft.VisualStudio.Component.VC.Tools.x86.x64','-property','installationPath']).decode().strip()
    dev=Path(install)/'Common7/Tools/VsDevCmd.bat'
    for name in ('fixture','windows-canary'):
        script=output/(name+'.cmd')
        script.write_text('@echo off\r\ncall "'+str(dev)+'" -arch=x86 -host_arch=x64 > "'+str(output/(name+'-environment.log'))+'" 2>&1\r\nif errorlevel 1 exit /b %errorlevel%\r\ncl /nologo /W4 /WX /Od /MT /TC "'+str(output/(name+'.c'))+'" /Fo:"'+str(output/(name+'.obj'))+'" /Fe:"'+str(output/(name+'.exe'))+'"\r\nexit /b %errorlevel%\r\n',encoding='utf-8',newline='')
        command(['cmd.exe','/d','/c',str(script)],output,name+'-compile.log')
    policy_log=command([str(output/'fixture.exe')],output,'fixture-run.log')
    canary_log=command([str(output/'windows-canary.exe')],output,'windows-canary-run.log')
    result={'status':'PASS','source':str(source.resolve()),'source_sha256':sha(raw),
        'baseline_commit':BASELINE_COMMIT,'baseline_blob':BASELINE_BLOB,
        'policy':policy_log.strip(),'native_windows':canary_log.strip(),
        'actual_method_sha256':{s:sha(body(actual,s).encode())for s in signatures},
        'bootstrap_main_compiled':False,'real_window_actuators':False,'device_calls':0,'qualified':False}
    (output/'result.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(result,indent=2))

if __name__=='__main__':main()
