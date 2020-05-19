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
            self.get_output('Input appropriate number')
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
            counter = 0
            while counter < 100000:
                counter += 1
                try:
                    line = self.queue.get(timeout=0.1)
                except Empty:
                    if self.instance.poll() is not None:
                        exit()
                    if len(lines) == 0:
                        counter += 10000
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
            if counter >= 100000:
                self.instance.kill()
                print('Exiting. Conduit killed due to serious internal error!')
                exit()
            user_input = input()
            while not user_input:
                user_input = input()
            if extra_tools is not None and user_input in extra_tools:
                self.tools[int(user_input) - int(extra_tools[0])](self)
                extra_call = True
                user_input = '100'
            self.stdin.write(str.encode(user_input + '\n'))
            self.stdin.flush()

    def ask(self, request, marker=None):
        self.stdin.write(str.encode(request + '\n'))
        self.stdin.flush()
        return self.get_output(marker)

    def get_output(self, marker=None):
        lines = []
        marker_detected = False
        counter = 0
        while counter < 1000:
            try:
                if marker:
                    line = self.queue.get(timeout=0.001)
                    if marker in line:
                        marker_detected = True
                else:
                    line = self.queue.get(timeout=0.1)
            except Empty:
                if marker and not marker_detected:
                    counter += 1
                    continue
                if self.instance.poll() is not None:
                    return self.instance.returncode
                return lines
            else:
                lines.append(line)
        self.instance.kill()
        raise RuntimeError('Conduit error on internal request!')

    def get_members_list(self, info=True):
        if info:
            members = self.ask('2', 'Input element name')
        else:
            members = self.ask('1', 'Input element name')
        self.ask('-1', 'Input 1 for Yes and 0 for No')
        self.ask('1', 'Input appropriate number')
        return members

    def get_members_amount(self):
        members = self.ask('2', 'Input element name')
        self.ask('-1', 'Input 1 for Yes and 0 for No')
        self.ask('1', 'Input appropriate number')
        amount = 0
        for member in members:
            if len(member.split('\t')) >= 6:
                amount += 1
        return amount

    def get_member_data(self, i):
        self.ask('2', 'Input element name')
        data = self.ask(str(i), 'Input 1 for Yes and 0 for No')
        self.ask('1', 'Input appropriate number')
        return data

    def get_member_data_safe(self, member_id):
        self.ask('2', 'Input element name')
        data = self.ask(str(member_id), 'Input')
        for line in data:
            if 'Short name' in line:
                self.ask('1', 'Input appropriate number')
                return data
        self.ask('-1', 'Input 1 for Yes and 0 for No')
        self.ask('1', 'Input appropriate number')
        return None

    def find(self, request):
        self.ask('11', 'What do u search for')
        search_result = self.ask(request, 'Input 1 for Yes and 0 for No')
        self.ask('1', 'Input appropriate number')
        for line in search_result:
            if 'is made by' in line:
                return True, line
            if "Didn't find" in line:
                return False, None

    def add_post(self, member, link, date='0'):
        self.ask('1', 'Or input -1 to return')
        self.ask(member, 'Input date in format YYYY.MM.DD')
        self.ask(date, 'Input link')
        self.ask(link, 'Input 1 for Yes and 0 for No')
        self.ask('1', 'Input appropriate number')

    def get_posts_with_default_status(self):
        self.ask('8', 'Input appropriate number')
        posts = self.ask('0', 'Input number of the post')[1:-2]
        self.ask('-1', 'Input 1 for Yes and 0 for No')
        self.ask('1', 'Input appropriate number')
        return posts

    def get_approved_posts(self):
        self.ask('8', 'Input appropriate number')
        posts = self.ask('2', 'Input 1 for Yes and 0 for No')[1:-2]
        self.ask('1', 'Input appropriate number')
        return posts

    def approve_post(self, request):
        self.ask('8', 'Input appropriate number')
        self.ask('0', 'Input number of the post')
        self.ask(str(request), 'Input 1 for Yes and 0 for No')
        self.ask('1', 'Input appropriate number')

    def reject_post(self, request):
        self.ask('8', 'Input appropriate number')
        self.ask('1', 'Input number of the post')
        self.ask(str(request), 'Input 1 for Yes and 0 for No')
        self.ask('1', 'Input appropriate number')
