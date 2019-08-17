import sys

from tools import Tools

from subprocess import Popen, PIPE
from threading import Thread
from queue import Queue, Empty


class RegBook:
    def __init__(self, command_args, silent=True):
        executable = 'RegBook'
        if command_args.unix:
            executable = './RegBook'
        self.instance = Popen(executable, stdout=PIPE, stdin=PIPE, stderr=PIPE)
        self.stdin = self.instance.stdin
        self.stdout = self.instance.stdout
        self.queue = Queue()
        thread = Thread(target=self.async_read)
        thread.daemon = True
        thread.start()
        if silent:
            self.get_output()
        else:
            tools = Tools(command_args)
            self.tools = tools.funcs
            self.tools_names = tools.names
            self.communicate()

    def async_read(self):
        for line in self.stdout:
            self.queue.put(line.decode('utf-8'))
        self.stdout.close()

    @staticmethod
    def remove_mistake(lines):
        for line in lines:
            if 'mistake' in line:
                lines.remove(line)
                return

    @staticmethod
    def is_main_menu(lines):
        for line in lines:
            if 'Hello!' in line:
                return True
        return False

    @staticmethod
    def add_extra_tools(lines, tools):
        index = 0
        for i, line in enumerate(lines):
            if str(index) + '. ' in line:
                index += 1
            elif index > 0:
                indices = []
                for j, tool_name in enumerate(tools):
                    lines.insert(i + j, str(index + j) + '. ' + tool_name + '\n')
                    indices.append(str(index + j))
                return indices
        return None

    def communicate(self):
        extra_call = False
        while True:
            lines = []
            extra_tools = None
            while True:
                try:
                    line = self.queue.get(timeout=0.03)
                except Empty:
                    if self.instance.poll() is not None:
                        exit()
                    if len(lines) == 0:
                        continue
                    if extra_call:
                        self.remove_mistake(lines)
                        extra_call = False
                    if self.is_main_menu(lines):
                        extra_tools = self.add_extra_tools(lines, self.tools_names)
                    for line in lines:
                        sys.stdout.write(line)
                    break
                else:
                    lines.append(line)
            user_input = input()
            while not user_input:
                user_input = input()
            if extra_tools is not None and user_input in extra_tools:
                self.tools[int(user_input) - int(extra_tools[0])](self)
                extra_call = True
                user_input = '100'
            self.stdin.write(str.encode(user_input + '\n'))
            self.stdin.flush()

    def ask(self, request):
        self.stdin.write(str.encode(request + '\n'))
        self.stdin.flush()
        return self.get_output()

    def get_output(self):
        lines = []
        while True:
            try:
                line = self.queue.get(timeout=0.03)
            except Empty:
                if self.instance.poll() is not None:
                    return self.instance.returncode
                return lines
            else:
                lines.append(line)

    def get_members_list(self, info=True):
        if info:
            members = self.ask('2')
        else:
            members = self.ask('1')
        self.ask('-1')
        self.ask('1')
        return members

    def get_members_amount(self):
        members = self.ask('2')
        self.ask('-1')
        self.ask('1')
        amount = 0
        for member in members:
            if len(member.split('\t')) >= 6:
                amount += 1
        return amount

    def get_member_data(self, i):
        self.ask('2')
        data = self.ask(str(i))
        self.ask('1')
        return data

    def get_member_data_safe(self, member_id):
        self.ask('2')
        data = self.ask(str(member_id))
        for line in data:
            if 'Short name' in line:
                self.ask('1')
                return data
        self.ask('-1')
        self.ask('1')
        return None

    def find(self, request):
        self.ask('11')
        search_result = self.ask(request)
        self.ask('1')
        for line in search_result:
            if 'is made by' in line:
                return True, line
            if "Didn't find" in line:
                return False, None

    def add_post(self, member, link, date='0'):
        self.ask('1')
        self.ask(member)
        self.ask(date)
        self.ask(link)
        self.ask('1')

    def get_posts_with_default_status(self):
        self.ask('8')
        posts = self.ask('0')[1:-2]
        self.ask('-1')
        self.ask('1')
        return posts

    def get_approved_posts(self):
        self.ask('8')
        posts = self.ask('2')[1:-2]
        self.ask('1')
        return posts

    def approve_post(self, request):
        self.ask('8')
        self.ask('0')
        self.ask(str(request))
        self.ask('1')

    def reject_post(self, request):
        self.ask('8')
        self.ask('1')
        self.ask(str(request))
        self.ask('1')
