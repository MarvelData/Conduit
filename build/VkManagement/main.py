import sys
import vk

from collections import defaultdict
from subprocess import Popen, PIPE
from threading import Thread
from queue import Queue, Empty

class Vk:
    def __init__(self):
        print('Initializing vk api')
        self.api_version = '5.92'
        self.community_id = '17592208'
        self.user_api = None
        self.community_api = None
        if len(sys.argv) > 2:
            self.user_api = vk.API(vk.AuthSession('3116505', sys.argv[1], sys.argv[2], scope='wall, groups'))
        else:
            print('Not enough data to initialize vk user api')
        if len(sys.argv) > 3:
            self.community_api = vk.API(vk.Session(access_token=sys.argv[3]))
        else:
            print('Not enough data to initialize vk community api')

    def get_conversations(self, offset=0, count=200):
        if not self.community_api:
            return None
        return self.community_api.messages.getConversations(group_id=self.community_id, v=self.api_version, offset=offset, count=count, extended=1)

    def get_messages(self, conversation):
        if not self.community_api:
            return None
        return self.community_api.messages.getHistory(peer_id=conversation['conversation']['peer']['id'], v=self.api_version)

    def print_conversations(self, id):
        if not self.community_api:
            return None
        conversations = self.get_conversations()
        for item in conversations['items']:
            messages = self.get_messages(item)
            for message in messages['items']:
                if 'admin_author_id' in message and message['admin_author_id'] == id:
                    print(message)

    def get_posts(self, offset=0, count=100, filter='all'):
        if not self.user_api:
            return None
        return self.user_api.wall.get(owner_id='-' + self.community_id, v=self.api_version, offset=offset, count=count, filter=filter)

    def get_post_info(self, offset = 0, amount = 1, filter = 'all', i = 0):
        if not self.user_api:
            return None
        posts = self.get_posts(offset, amount, filter)
        for key, value in posts['items'][i].items():
            print(key, value)


class RegBook:
    def __init__(self, silent = True):
        self.instance = Popen(["Regbook"], stdout=PIPE, stdin=PIPE, stderr=PIPE)
        self.stdin = self.instance.stdin
        self.stdout = self.instance.stdout
        self.queue = Queue()
        thread = Thread(target=self.async_read)
        thread.daemon = True
        thread.start()
        if silent:
            self.get_output()
        else:
            tools = Tools()
            self.tools = tools.funcs
            self.tools_names = tools.names
            self.communicate()

    def async_read(self):
        for line in self.stdout:
            self.queue.put(line.decode('utf-8'))
        self.stdout.close()

    def remove_mistake(self, lines):
        for line in lines:
            if 'mistake' in line:
                lines.remove(line)
                return

    def is_main_menu(self, lines):
        for line in lines:
            if 'Hello!' in line:
                return True
        return False

    def add_extra_tools(self, lines):
        index = 0
        for i, line in enumerate(lines):
            if str(index) + '. ' in line:
                index += 1
            elif index > 0:
                indices = []
                for j, tool_name in enumerate(self.tools_names):
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
                    if extra_call:
                        self.remove_mistake(lines)
                        extra_call = False
                    if self.is_main_menu(lines):
                        extra_tools = self.add_extra_tools(lines)
                    for line in lines:
                        sys.stdout.write(line)
                    if self.instance.poll() is not None:
                        exit()
                    break
                else:
                    lines.append(line)
            user_input = input()
            if extra_tools is not None and user_input in extra_tools:
                self.tools[int(user_input) - int(extra_tools[0])]()
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

    def find(self, request):
        self.ask('11')
        search_result = self.ask(request)
        self.ask('1')
        for line in search_result:
            if 'is posted by' in line:
                return True, line
            if 'Didn\'t find' in line:
                return False, None

    def add_post(self, member, link, date = '0'):
        self.ask('1')
        self.ask(member)
        self.ask(date)
        self.ask(link)
        self.ask('1')

class Tools:
    def __init__(self):
        self.vk = None
        self.funcs = []
        self.funcs.append(self.process_postponed_posts)
        self.funcs.append(self.process_posts)
        self.funcs.append(self.get_stats)
        self.names = []
        self.names.append('I would like to autoadd posts for members')
        self.names.append('I would like to automanage posts')
        self.names.append('I would like to get stats for members')

    def init_vk(self):
        if not self.vk:
            self.vk = Vk()

    def post_id_to_link(self, id):
        return 'https://vk.com/wall-17592208_' + id

    def get_hashtag(self, post):
        hashtag = post['text'].split('\n')[0]
        if '#' in hashtag:
            return hashtag
        else:
            return '#' + hashtag

    def rubric_to_hashtag(self, rubric):
        return '#MU_' + rubric

    def list_to_string(self, list):
        string = ''
        for elem in list:
            string += elem + ', '
        return string[:-2]

    def parse_member_data(self, data):
        member = {}
        for line in data:
            if 'Short name:' in line:
                member['name'] = line.split(':')[1][1:-2]
            if 'Rubric:' in line:
                member['rubric'] = line.split(':')[1][1:-2]
            if len(line) > 5 and not ' ' in line:
                member['id'] = line[:-3]
        return member

    def get_members_data(self, regBook):
        key_to_member = {}
        id_to_members = defaultdict(list)
        member_amount = regBook.get_members_amount()
        for i in range(member_amount):
            member = self.parse_member_data(regBook.get_member_data(i))
            key_to_member[member['id'] + self.rubric_to_hashtag(member['rubric'])] = member['name']
            id_to_members[member['id']].append(member['name'])
        return key_to_member, id_to_members

    def process_postponed_posts(self):
        self.init_vk()
        regBook = RegBook()
        key_to_member, id_to_members = self.get_members_data(regBook)
        posts = self.vk.get_posts(0, 100, 'postponed ')
        if not posts:
            regBook.ask('-1')
            return
        print('\nThere are', posts['count'], 'postponed posts\n')
        counter = 0
        not_found, need_work = [], []
        for post in posts['items']:
            if 'created_by' in post and str(post['created_by']) in id_to_members:
                id = str(post['created_by'])
                key = id + self.get_hashtag(post)
                link = self.post_id_to_link(str(post['id']))
                found, result = regBook.find(link)
                if found:
                    counter += 1
                if not found:
                    if key in key_to_member:
                        regBook.add_post(key_to_member[key], link)
                        not_found.append(key_to_member[key] + ' ' + link + '\nAdded to Conduit')
                    else:
                        not_found.append('Incomplete key: ' + key + '\nPossible authors of ' + link + ' -- ' + self.list_to_string(id_to_members[id]))
                elif not key in key_to_member:
                    need_work.append(result)
        print(len(not_found), 'posts not found in Conduit:\n')
        for post in not_found:
            print(post)
            print()
        print()
        print(counter, 'posts already in Conduit')
        print(len(need_work), 'out of them have to be finished:\n')
        for post in need_work:
            print(post)
        print('Silent RegBook returns', regBook.ask('-1'))

    def process_posts(self):
        self.init_vk()
        print('\nprocess_posts')

    def get_posts_stats(self, amount, id, rubric):
        self.init_vk()
        stats = defaultdict(list)
        counter = 0
        offset = -100
        while counter < amount:
            offset += 100
            posts = self.vk.get_posts(offset, 100)
            if not posts:
                return
            for post in posts['items']:
                if 'signer_id' in post and post['signer_id'] == id and self.get_hashtag(post) == rubric:
                    for stat_type in ['comments', 'likes', 'reposts', 'views']:
                        stats[stat_type].append(post[stat_type]['count'])
                    stats['efficiency'].append(post['likes']['count'] / post['views']['count'])
                    counter += 1
                    if counter >= amount:
                        break
        for stat_type, values in stats.items():
            sum = 0
            for value in values:
                sum += value
            print(stat_type, sum / counter)

    def get_stats(self):
        print()
        self.get_posts_stats(50, id, scenes)

id = 356508400
scenes = '#MU_Scenes'

RegBook(silent=False)