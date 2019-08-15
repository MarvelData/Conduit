import argparse
import os
import sys
import vk

from collections import defaultdict
from datetime import datetime
from datetime import timedelta
from matplotlib import pyplot
from subprocess import Popen, PIPE
from threading import Thread
from time import sleep
from queue import Queue, Empty


class Vk:
    def __init__(self):
        print('\nInitializing vk api')
        self.api_version = '5.101'
        self.community_id = '17592208'
        self.admins = [5962550, 155003349, 151414919, 295683611]
        self.user_api = None
        self.community_api = None
        if command_args.login and command_args.password:
            self.user_api = vk.API(vk.AuthSession('3116505', command_args.login, command_args.password, scope='wall, groups'))
        else:
            print('Not enough data to initialize vk user api')
        if command_args.token:
            self.community_api = vk.API(vk.Session(access_token=command_args.token))
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
        conversations = self.get_conversations()
        for item in conversations['items']:
            messages = self.get_messages(item)
            for message in messages['items']:
                if 'admin_author_id' in message and message['admin_author_id'] == id:
                    print(message)

    def get_posts(self, offset=0, count=100, filter='all', id=None):
        if not self.user_api:
            return None
        if not id:
            id = '-' + self.community_id
        try:
            return self.user_api.wall.get(owner_id=id, v=self.api_version, offset=offset, count=count, filter=filter)
        except:
            sleep(0.5)
            return self.get_posts(offset, count, filter)

    def get_post(self, id):
        if not self.user_api:
            return None
        post_id = '-' + self.community_id + '_' + str(id)
        try:
            return self.user_api.wall.getById(posts=post_id, v=self.api_version)
        except:
            sleep(0.5)
            return self.get_post(id)

    def get_reposts(self, id):
        if not self.user_api:
            return None
        return self.user_api.likes.getList(type='post', owner_id='-' + self.community_id, item_id=id, v=self.api_version, filter='copies')

    def get_post_info(self, offset = 0, amount = 1, filter = 'all', i = 0):
        if not self.user_api:
            return None
        posts = self.get_posts(offset, amount, filter)
        for key, value in posts['items'][i].items():
            print(key, value)


class RegBook:
    def __init__(self, silent = True):
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

    def add_extra_tools(self, lines, tools):
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
        self.instance.kill()
        return None

    def find(self, request):
        self.ask('11')
        search_result = self.ask(request)
        self.ask('1')
        for line in search_result:
            if 'is made by' in line:
                return True, line
            if 'Didn\'t find' in line:
                return False, None

    def add_post(self, member, link, date = '0'):
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


class Tools:
    def __init__(self):
        self.vk = None
        self.funcs = []
        self.funcs.append(self.process_postponed_posts)
        self.funcs.append(self.process_posts)
        self.funcs.append(self.get_member_stats)
        self.funcs.append(self.get_member_plot)
        self.funcs.append(self.get_members_plot)
        self.funcs.append(self.get_specific_posts)
        self.funcs.append(self.get_postponed_link_for_post)
        self.funcs.append(self.get_actual_link_for_post)
        self.names = []
        self.names.append('I would like to autoadd posts for members')
        self.names.append('I would like to automanage posts')
        self.names.append('I would like to get stats for member')
        self.names.append('I would like to get plot for member')
        self.names.append('I would like to get plot for all members')
        self.names.append('I would like to get specific posts')
        self.names.append('I would like to get postponed link for post')
        self.names.append('I would like to get actual link for post')

    def init_vk(self):
        if not self.vk:
            self.vk = Vk()
        return self.vk.user_api

    def my_date_to_universal(self, date):
        return datetime.strptime(date, '%Y.%m.%d')

    def vk_date_to_universal(self, date):
        return datetime.fromtimestamp(date)

    def post_id_to_link(self, id, prefix='https://vk.com/wall-17592208_'):
        return prefix + str(id)

    def post_id_to_cached_link(self, id):
        return 'https://vk.com/mu_marvel?w=wall-17592208_' + str(id)

    def get_hashtag(self, post):
        hashtag = post['text'].split('\n')[0]
        if '#' in hashtag:
            return hashtag
        else:
            return '#' + hashtag

    def rubric_to_hashtag(self, rubric):
        return '#MU_' + rubric

    def clear_string(self, string):
        return string.replace(' ', '').replace('\n', '').replace('\r', '')

    def list_to_string(self, list):
        string = ''
        for elem in list:
            string += elem + ', '
        return string[:-2]

    def days_list_to_plot_ticks(self, days):
        days_to_show, dates_to_show = [], []
        last_shown_day = -sys.maxsize
        for i, day_number in enumerate(days):
            if day_number - last_shown_day > (days[-1] - days[0]) / 9:
                last_shown_day = day_number
                days_to_show.append(days[i])
                dates_to_show.append((self.my_date_to_universal('2019.01.01') + timedelta(days=days[i])).date())
        return days_to_show, dates_to_show

    def parse_member_data(self, data):
        member = {}
        posts = {}
        for line in data:
            if 'Short name:' in line:
                member['name'] = self.clear_string(line.split(':')[1])
            if 'Rubric:' in line:
                member['rubric'] = self.clear_string(line.split(':')[1])
            if len(line) > 5 and not ' ' in line:
                member['id'] = self.clear_string(line.split('\\')[0])
            if 'http' in line:
                columns = line.split(' ')
                amount = int(columns[1])
                for i in range(0, amount):
                    posts[self.clear_string(columns[2 + i * 2])] = \
                         [self.clear_string(columns[0]), self.clear_string(columns[3 + i * 2])]
        member['posts'] = posts
        return member

    def get_members_mapping(self, regBook):
        key_to_member = {}
        id_to_members = defaultdict(list)
        member_amount = regBook.get_members_amount()
        for i in range(member_amount):
            member = self.parse_member_data(regBook.get_member_data(i))
            key_to_member[member['id'] + self.rubric_to_hashtag(member['rubric'])] = member['name']
            id_to_members[member['id']].append(member['name'])
        return key_to_member, id_to_members

    def get_member_posts_dates(self, data):
        days, amounts = [], []
        date_from = self.my_date_to_universal('2019.01.01')
        for line in data:
            if 'http' in line:
                columns = line.split(' ')
                days.append((self.my_date_to_universal(self.clear_string(columns[0])) - date_from).days)
                amounts.append(int(self.clear_string(columns[1])))
        return days, amounts

    def choose_member(self, regBook, info=True, stats=False):
        members = regBook.get_members_list(info)
        extra_tools = None
        if stats:
            extra_tools = regBook.add_extra_tools(members, ['I would like to perform custom request'])
        for line in members:
            sys.stdout.write(line)
        while True:
            user_input = input()
            if extra_tools is not None and user_input in extra_tools:
                custom_data = []
                print('\nInput short name\n')
                custom_data.append('Short name: ' + input())
                print('\nInput rubric\n')
                custom_data.append('Rubric: ' + input())
                print('\nInput id\n')
                custom_data.append(input())
                return custom_data, regBook
            if user_input == '-1':
                return None, regBook
            member_data = regBook.get_member_data_safe(user_input)
            if not member_data:
                regBook = RegBook()
                print('\nNo such element! Try again.\n')
            else:
                break
        for line in member_data[:-3]:
            sys.stdout.write(line)
        return member_data, regBook

    def filter_Kamil(self, member, post):
        banned_list = [398504693, -134362257]
        fake_likes, fake_reposts, fake_views = 0, 0, 0
        if 'Kamil' in member['name']:
            sleep(0.5)
            reposters = self.vk.get_reposts(post['id'])
            for reposter in reposters['items']:
                if reposter in banned_list:
                    print(reposter)
                    reposter_posts = self.vk.get_posts(id=reposter)
                    for reposter_post in reposter_posts['items']:
                        if 'copy_history' in reposter_post and reposter_post['copy_history'][0]['id'] == post['id']:
                            print(self.post_id_to_link(reposter_post['id'], prefix='https://vk.com/wall' + str(reposter) + '_'),
                                  reposter_post['likes']['count'], reposter_post['reposts']['count'])
                            fake_likes += reposter_post['likes']['count']
                            fake_reposts += reposter_post['reposts']['count']
                            fake_views += reposter_post['views']['count']
            print()
        return fake_likes, fake_reposts, fake_views

    def process_postponed_posts(self):
        if not self.init_vk():
            return
        regBook = RegBook()
        key_to_member, id_to_members = self.get_members_mapping(regBook)
        print('\nInput date to add posts from in format YYYY.MM.DD or "0" for basic option\n')
        date_from = input()
        print()
        if date_from == '0':
            date_from = datetime.now() - timedelta(days=1)
            max_posts = 4
            step = 1
        else:
            date_from = self.my_date_to_universal(date_from)
            print('Input max posts amount per day\n')
            max_posts = int(input())
            print()
            print('Input step\n')
            step = int(input())
            print()
        member_to_posts = {}
        for _, member in key_to_member.items():
            member_to_posts[member] = [0, date_from]
        posts = self.vk.get_posts(filter='postponed')
        if posts['count'] > 100:
            new_posts = self.vk.get_posts(offset=100, filter='postponed')
            posts['items'] += new_posts['items']
        print('\nThere are', posts['count'], 'postponed posts\n')
        counter = 0
        not_found, need_work = [], []
        for post in posts['items']:
            if 'created_by' in post and str(post['created_by']) in id_to_members:
                id = str(post['created_by'])
                key = id + self.get_hashtag(post).replace(' ', '')
                link = self.post_id_to_link(post['id'])
                found, result = regBook.find(link)
                if found:
                    counter += 1
                if not found:
                    if key in key_to_member:
                        member = key_to_member[key]
                        member_to_posts[member][0] += 1
                        if member_to_posts[member][0] > max_posts:
                            member_to_posts[member][1] += timedelta(days=step)
                            member_to_posts[member][0] = 0
                        regBook.add_post(member, link, member_to_posts[member][1].strftime('%Y.%m.%d'))
                        not_found.append(member + ' ' + link + '\nAdded to Conduit')
                    else:
                        not_found.append('Incomplete hashtag: ' + self.get_hashtag(post) + '\nPossible authors of ' +
                                         link + ' -- ' + self.list_to_string(id_to_members[id]))
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
        if regBook.ask('-1') != 0:
            print('\nWarning: Silent RegBook did not exit correctly')

    def process_posts(self):
        if not self.init_vk():
            return
        regBook = RegBook()
        key_to_member, id_to_members = self.get_members_mapping(regBook)
        posts_to_judge = regBook.get_posts_with_default_status()
        approved_posts = regBook.get_approved_posts()
        mapped_posts = {}
        for post in posts_to_judge:
            mapped_posts[self.clear_string(post.split(' ')[-1])] = False
        for post in approved_posts:
            mapped_posts[self.clear_string(post.split(' ')[-1])] = True
        old_posts = {}
        for file in os.listdir(os.fsencode('../data')):
            filename = os.fsdecode(file)
            if '_dismissed' in filename:
                contents = open('../data/' + filename).readlines()
                for line in contents:
                    if 'https' in line:
                        columns = line.replace('\t', ' ').split(' ')
                        for text in columns:
                            if 'https' in text:
                                old_posts[self.clear_string(text)] = False
        date_from = self.my_date_to_universal('2019.07.01')
        print()
        counter = 0
        actual_links_counter = 0
        offset = -100
        date = datetime.now()
        while date >= date_from:
            offset += 100
            posts = self.vk.get_posts(offset, 100)
            for post in posts['items']:
                date = self.vk_date_to_universal(post['date'])
                if date < date_from:
                    continue
                signer_id = None
                link, actual_link = None, self.post_id_to_cached_link(post['id'])
                easy_link = self.post_id_to_link(post['id'])
                hashtag = self.get_hashtag(post)
                if 'signer_id' in post and str(post['signer_id']) in id_to_members:
                    signer_id = str(post['signer_id'])
                if 'postponed_id' in post:
                    link = self.post_id_to_cached_link(post['postponed_id'])
                if link and link in old_posts or actual_link in old_posts:
                    continue
                if link and link in mapped_posts:
                    if mapped_posts[link]:
                        continue
                    regBook.approve_post(link)
                    mapped_posts[link] = True
                    continue
                if actual_link in mapped_posts:
                    print('Actual link in conduit:', actual_link)
                    actual_links_counter += 1
                    if mapped_posts[actual_link]:
                        continue
                    regBook.approve_post(actual_link)
                    mapped_posts[actual_link] = True
                    continue
                if '#MU_Paintings' in hashtag or '#MU_PartnersVideo' in hashtag\
                or '#MU_Anouncements' in hashtag or '#MU_Special' in hashtag:
                    continue
                if not post['text'] or post['text'][0] != '#':
                    print('Specific post', easy_link)
                    continue
                if not link:
                    print('Not postponed', easy_link)
                    continue
                if signer_id:
                    key = signer_id + hashtag.replace(' ', '')
                    if key in key_to_member:
                        print(key_to_member[key], 'posted', easy_link, '(key:', key + ')', date.strftime('%Y.%m.%d'))
                        regBook.add_post(key_to_member[key], link, (date - timedelta(days=3)).strftime('%Y.%m.%d'))
                        regBook.approve_post(link)
                    else:
                        print('One of', id_to_members[signer_id], 'posted', easy_link, hashtag)
                else:
                    if not 'signer_id' in post:
                        print('Unknown post', easy_link, hashtag)
                        counter += 1
                    elif not post['signer_id'] in self.vk.admins:
                        print('Unaccounted post by', 'https://vk.com/id' + str(post['signer_id']), easy_link, hashtag)
        print()
        print(counter, 'unknown posts,', actual_links_counter, 'actual links in conduit')
        for link, found in mapped_posts.items():
            if not found and len(self.vk.get_post(link.split('_')[-1])) == 0:
                regBook.reject_post(link)
        if regBook.ask('-1') != 0:
            print('\nWarning: Silent RegBook did not exit correctly')

    def get_member_stats(self):
        if not self.init_vk():
            return
        regBook = RegBook()
        member_data, regBook = self.choose_member(regBook, info=True, stats=True)
        if regBook.ask('-1') != 0:
            print('\nWarning: Silent RegBook did not exit correctly')
        if not member_data:
            return
        member = self.parse_member_data(member_data)
        print('Input date to collect stats from in format YYYY.MM.DD\n')
        date_from = self.my_date_to_universal(input())
        print()
        print('Input date to collect stats to in format YYYY.MM.DD\n')
        date_to = self.my_date_to_universal(input())
        print()
        id = int(member['id'])
        rubric = self.rubric_to_hashtag(member['rubric'])
        stats = defaultdict(list)
        counter = 0
        offset = -100
        date = datetime.now()
        while date >= date_from:
            offset += 100
            posts = self.vk.get_posts(offset, 100)
            for post in posts['items']:
                date = self.vk_date_to_universal(post['date'])
                if date >= date_to:
                    continue
                if date < date_from:
                    continue
                if 'signer_id' in post and post['signer_id'] == id and self.get_hashtag(post).replace(' ', '') == rubric\
                or 'postponed_id' in post and self.post_id_to_cached_link(post['postponed_id']) in member['posts']\
                or self.post_id_to_cached_link(post['id']) in member['posts']:
                    print(date, self.post_id_to_link(post['id']), post['likes']['count'], post['reposts']['count'])
                    fake_likes, fake_reposts, fake_views = self.filter_Kamil(member, post)
                    for stat_type in ['comments', 'likes', 'reposts', 'views']:
                        stats[stat_type].append(post[stat_type]['count'])
                    stats['efficiency'].append((post['likes']['count'] - fake_likes) / (post['views']['count'] - fake_views))
                    stats['likes'][-1] -= fake_likes
                    stats['reposts'][-1] -= fake_reposts
                    stats['views'][-1] -= fake_views
                    counter += 1
        made_counter = 0
        for post, info in member['posts'].items():
            date = self.my_date_to_universal(info[0])
            if date >= date_from and date < date_to and info[1] != '!':
                made_counter += 1
        print()
        print(member['name'], 'made', made_counter, 'posts since', date_from.date())
        print(member['name'], 'has', counter, 'released posts since', date_from.date())
        print(member['name'] + "'s", 'average results are:')
        for stat_type, values in stats.items():
            sum = 0
            for value in values:
                sum += value
            print(stat_type + ':', sum / counter)

    def get_member_plot(self):
        regBook = RegBook()
        member_data, regBook = self.choose_member(regBook, info=True)
        if regBook.ask('-1') != 0:
            print('\nWarning: Silent RegBook did not exit correctly')
        if not member_data:
            return
        days, amounts = self.get_member_posts_dates(member_data)
        new_days = [days[0]]
        new_amounts = [0]
        for i, day in enumerate(days):
            if day >= new_days[-1] + 10:
                new_days.append(new_days[-1] + 10)
                new_amounts.append(0)
            if day < new_days[-1] + 10:
                new_amounts[len(new_days) - 1] += amounts[i]
        pyplot.xlabel('Dates')
        pyplot.ylabel('Posts')
        pyplot.grid(True)
        pyplot.plot(new_days, new_amounts, 'g', linewidth='1')
        pyplot.plot(new_days, new_amounts, 'bx', markersize='7')
        days_to_show, dates_to_show = self.days_list_to_plot_ticks(new_days)
        pyplot.xticks(days_to_show, dates_to_show)
        pyplot.yticks(new_amounts, new_amounts)
        pyplot.show()

    def get_members_plot(self):
        regBook = RegBook()
        member_amount = regBook.get_members_amount()
        pyplot.xlabel('Dates')
        pyplot.ylabel('Posts')
        pyplot.grid(True)
        all_days, all_amounts = [], []
        members_posts = {}
        member_days = {}
        for i in range(member_amount):
            member_data = regBook.get_member_data(i)
            days, amounts = self.get_member_posts_dates(member_data)
            member = self.parse_member_data(member_data)
            all_days += days
            members_posts[member['name']] = amounts
            member_days[member['name']] = days
        all_days = sorted(all_days)
        if regBook.ask('-1') != 0:
            print('\nWarning: Silent RegBook did not exit correctly')
        saved_new_days = []
        i = 0
        for member, posts in members_posts.items():
            new_days = [all_days[0]]
            new_amounts = [0]
            for i, day in enumerate(member_days[member]):
                if day >= new_days[-1] + 10:
                    new_days.append(new_days[-1] + 10)
                    new_amounts.append(0)
                if day < new_days[-1] + 10:
                    new_amounts[len(new_days) - 1] += posts[i]
            all_amounts += new_amounts
            if len(new_days) > len(saved_new_days):
                saved_new_days = new_days
            pyplot.plot(new_days, new_amounts, marker='x', linewidth=0.5, markersize='7', label=member)
            i += 1
        days_to_show, dates_to_show = self.days_list_to_plot_ticks(saved_new_days)
        pyplot.xticks(days_to_show, dates_to_show)
        pyplot.yticks(all_amounts, all_amounts)
        pyplot.legend()
        pyplot.show()

    def get_specific_posts(self):
        if not self.init_vk():
            return
        offset = -100
        date = datetime.now()
        date_from = self.my_date_to_universal('2019.06.01')
        print()
        while date >= date_from:
            offset += 100
            posts = self.vk.get_posts(offset, 100)
            for post in posts['items']:
                date = self.vk_date_to_universal(post['date'])
                if date < date_from:
                    continue
                if not post['text'] or post['text'][0] != '#':
                    print(self.post_id_to_link(post['id']))

    def get_postponed_link_for_post(self):
        if not self.init_vk():
            return
        print('\nInput your link\n')
        posts = self.vk.get_post(self.clear_string(input()).split('_')[-1])
        if len(posts) == 0:
            print('\nWrong link')
            return
        if len(posts) > 1:
            print('\nSomething went wrong, you got a lot of posts:', len(posts))
            return
        post = posts[0]
        if 'postponed_id' in post:
            print()
            print(self.post_id_to_cached_link(post['postponed_id']))
        else:
            print('\nThis post was not postponed!')

    def get_actual_link_for_post(self):
        if not self.init_vk():
            return
        print('\nInput your link\n')
        postponed_id = int(self.clear_string(input()).split('_')[-1])
        print('\nInput date to search back to in format YYYY.MM.DD\n')
        date_from = self.my_date_to_universal(input())
        print()
        offset = -100
        date = datetime.now()
        while date >= date_from:
            offset += 100
            posts = self.vk.get_posts(offset, 100)
            for post in posts['items']:
                date = self.vk_date_to_universal(post['date'])
                if date < date_from:
                    continue
                if 'postponed_id' in post and post['postponed_id'] == postponed_id:
                    print(self.post_id_to_link(post['id']))
                    return
        print('Post not found in time period since', date_from.strftime('%Y.%m.%d'))


parser = argparse.ArgumentParser()
parser.add_argument('--login', default='')
parser.add_argument('--password', default='')
parser.add_argument('--token', default='')
parser.add_argument('--unix', default='')
command_args = parser.parse_args()

executable = 'RegBook'
if command_args.unix:
    executable = './RegBook'
RegBook(silent=False)
