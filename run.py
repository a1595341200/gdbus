from email.policy import default
from hashlib import new
import os
import shutil
import sys
import subprocess

class build_project:
    def __init__(self) -> None:
        pass
            
    def cmd(self,cmd_line):
        if(self.get_platform() == "linux"):
            command = cmd_line
            out = subprocess.getoutput(command)
        print(out)

    def get_platform(self):
        print(sys.platform)
        return sys.platform

    def codegen(self):
            if(self.get_platform() == "linux"):
                command = 'gdbus-codegen --generate-c-code=Test --c-namespace gdbus --interface-prefix com.yao.xie.gdbus. framework/interface/Test.xml'
                self.cmd(command)
                self.mymovefile("Test.h","framework/include")
                self.mymovefile("Test.c","framework/include")
    # srcfile 需要复制、移动的文件   
    # dstpath 目的地址
    
    def mymovefile(self,srcfile,dstpath):                       # 移动函数
        if not os.path.isfile(srcfile):
            print ("%s not exist!"%(srcfile))
        else:
            fpath,fname=os.path.split(srcfile)             # 分离文件名和路径
            if not os.path.exists(dstpath):
                os.makedirs(dstpath)                       # 创建路径
            shutil.move(srcfile, dstpath + fname)          # 移动文件
            print ("move %s -> %s"%(srcfile, dstpath + fname))
    def build(self):
        if(self.get_platform() == "linux"):
            command = 'cmake -B build'
            self.cmd(command)

    def make(self):
        if(self.get_platform() == "linux"):
            command = 'make -C build'
            self.cmd(command)

if __name__ == '__main__':
        build = build_project()
        build.codegen()
        build.build()
        build.make()
