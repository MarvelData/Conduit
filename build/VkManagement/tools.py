import os
import sys

from utils import Utils
from vkConnector import Vk

from collections import defaultdict
from datetime import datetime
from datetime import timedelta
from matplotlib import pyplot


class Tools:
    def __init__(self, command_args):
        self.command_args = command_args
        self.vk = None
        self.funcs = []
        self.funcs.append(self.process_postponed_posts)
        self.funcs.append(self.process_posts)
        self.funcs.append(self.get_member_stats)
        self.funcs.append(self.analyze_rubrics)
        self.funcs.append(self.get_member_plot)
        self.funcs.append(self.get_members_plot)
        self.funcs.append(self.get_postponed_link_for_post)
        self.funcs.append(self.get_actual_link_for_post)
        self.names = []
        self.names.append('I would like to autoadd posts for members')
        self.names.append('I would like to automanage posts')
        self.names.append('I would like to get stats for member')
        self.names.append('I would like to analyze rubrics stats')
        self.names.append('I would like to get plot for member')
        self.names.append('I would like to get plot for all members')
        self.names.append('I would like to get postponed link for post')
        self.names.append('I would like to get actual link for post')

    def init_vk(self):
        if not self.vk:
            self.vk = Vk(self.command_args)
        return self.vk.user_api

    def process_postponed_posts(self, regBook):
        if not self.init_vk():
            return
        key_to_member, id_to_members = Utils.get_members_mapping(regBook)
        print('\nInput date to add posts from in format YYYY.MM.DD or "0" for basic option\n')
        date_from = input()
        print()
        if date_from == '0':
            date_from = datetime.now() - timedelta(days=1)
            max_posts = 4
            step = 1
        else:
            date_from = Utils.my_date_to_universal(date_from)
            print('Input max posts amount per day\n')
            max_posts = int(input())
            print()
            print('Input step\n')
            step = int(input())
            print()
        member_to_posts = {}
        for _, member in key_to_member.items():
            member_to_posts[member] = [0, date_from]
        posts = self.vk.get_posts(type_filter='postponed')
        if posts['count'] > 100:
            new_posts = self.vk.get_posts(offset=100, type_filter='postponed')
            posts['items'] += new_posts['items']
        print('\nThere are', posts['count'], 'postponed posts\n')
        counter = 0
        not_found, need_work = [], []
        content = defaultdict(list)
        for post in posts['items']:
            if 'created_by' in post and str(post['created_by']) in id_to_members:
                editor_id = str(post['created_by'])
                key = editor_id + Utils.get_hashtag(post).replace(' ', '')
                link = Utils.post_id_to_link(post['id'])
                found, result = regBook.find(link)
                if found:
                    counter += 1
                    name = Utils.clear_string(result.split(' ')[-1])
                    content[Utils.parse_member_data(regBook.get_member_data(name))['rubric']].append(link)
                if not found:
                    if key in key_to_member:
                        member = key_to_member[key]
                        member_to_posts[member][0] += 1
                        if member_to_posts[member][0] > max_posts:
                            member_to_posts[member][1] += timedelta(days=step)
                            member_to_posts[member][0] = 1
                        regBook.add_post(member, link, member_to_posts[member][1].strftime('%Y.%m.%d'))
                        not_found.append(member + ' ' + link + '\nAdded to Conduit')
                    else:
                        not_found.append('Incomplete hashtag: ' + Utils.get_hashtag(post) + '\nPossible authors of ' +
                                         link + ' -- ' + Utils.list_to_string(id_to_members[editor_id]))
                elif key not in key_to_member:
                    need_work.append(result)
            else:
                link = Utils.post_id_to_link(post['id'])
                found, result = regBook.find(link)
                if found:
                    counter += 1
                    sys.stdout.write(result)
                if 'created_by' in post:
                    print(link, 'is posted by', 'https://vk.com/id' + str(post['created_by']))
                else:
                    print(link, 'is posted by', '???')
                print()
        print(len(not_found), 'posts not found in Conduit:\n')
        for post in not_found:
            print(post)
            print()
        print()
        print(counter, 'posts already in Conduit')
        print(len(need_work), 'out of them have to be finished:\n')
        for post in need_work:
            print(post)
        for rubric, posts in sorted(content.items()):
            print(len(posts), 'of type', rubric)

    def process_posts(self, regBook):
        if not self.init_vk():
            return
        key_to_member, id_to_members = Utils.get_members_mapping(regBook)
        posts_to_judge = regBook.get_posts_with_default_status()
        approved_posts = regBook.get_approved_posts()
        mapped_posts = {}
        for post in posts_to_judge:
            mapped_posts[Utils.clear_string(post.split(' ')[-1])] = False
        for post in approved_posts:
            mapped_posts[Utils.clear_string(post.split(' ')[-1])] = True
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
                                old_posts[Utils.clear_string(text)] = False
        print('\nInput date to process posts from in format YYYY.MM.DD\n')
        date_from = Utils.my_date_to_universal(input())
        print()
        counter = 0
        actual_links_counter = 0
        offset = -100
        date = datetime.now()
        while date >= date_from:
            offset += 100
            posts = self.vk.get_posts(offset, 100)
            for post in posts['items']:
                date = Utils.vk_date_to_universal(post['date'])
                if date < date_from:
                    continue
                signer_id = None
                link, actual_link = None, Utils.post_id_to_cached_link(post['id'])
                easy_link = Utils.post_id_to_link(post['id'])
                hashtag = Utils.get_hashtag(post)
                if 'signer_id' in post and str(post['signer_id']) in id_to_members:
                    signer_id = str(post['signer_id'])
                if 'postponed_id' in post:
                    link = Utils.post_id_to_cached_link(post['postponed_id'])
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
                or '#MU_Anouncements' in hashtag or '#MU_Announcements' in hashtag\
				or '#MU_Special' in hashtag:
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
                    if 'signer_id' not in post:
                        print('Unknown post', easy_link, hashtag)
                        counter += 1
                    elif not post['signer_id'] in self.vk.admins:
                        print('Unaccounted post by', 'https://vk.com/id' + str(post['signer_id']), easy_link, hashtag)
        print()
        print(counter, 'unknown posts,', actual_links_counter, 'actual links in conduit')
        for link, found in mapped_posts.items():
            if not found and len(self.vk.get_post(link.split('_')[-1])) == 0:
                regBook.reject_post(link)

    def get_member_stats(self, regBook):
        if not self.init_vk():
            return
        member_data = Utils.choose_member(regBook, info=True, stats=True)
        if not member_data:
            return
        member = Utils.parse_member_data(member_data)
        print('Input date to collect stats from in format YYYY.MM.DD\n')
        date_from = Utils.my_date_to_universal(input())
        print()
        print('Input date to collect stats to in format YYYY.MM.DD\n')
        date_to = Utils.my_date_to_universal(input())
        print()
        editor_id = int(member['id'])
        rubric = Utils.rubric_to_hashtag(member['rubric'])
        stats = defaultdict(list)
        counter = 0
        offset = -100
        date = datetime.now()
        while date >= date_from:
            offset += 100
            posts = self.vk.get_posts(offset, 100)
            for post in posts['items']:
                date = Utils.vk_date_to_universal(post['date'])
                if date >= date_to:
                    continue
                if date < date_from:
                    continue
                hashtag = Utils.get_hashtag(post).replace(' ', '')
                if 'signer_id' in post and post['signer_id'] == editor_id and hashtag == rubric\
                or 'postponed_id' in post and Utils.post_id_to_cached_link(post['postponed_id']) in member['posts']\
                or Utils.post_id_to_cached_link(post['id']) in member['posts']:
                    votes = -1
                    if 'attachments' in post and 'poll' in post['attachments'][0]:
                        votes = self.vk.get_poll(post['attachments'][0]['poll']['id'])['votes']
                    print(date, Utils.post_id_to_link(post['id']), post['likes']['count'], post['reposts']['count'], votes)
                    fake_likes, fake_reposts, fake_views = Utils.filter_Kamil(member, post, self.vk)
                    for stat_type in ['comments', 'likes', 'reposts', 'views']:
                        stats[stat_type].append(post[stat_type]['count'])
                    stats['efficiency'].append((post['likes']['count'] - fake_likes)
                                             / (post['views']['count'] - fake_views))
                    stats['likes'][-1] -= fake_likes
                    stats['reposts'][-1] -= fake_reposts
                    stats['views'][-1] -= fake_views
                    stats['votes'].append(votes)
                    counter += 1
        made_counter = 0
        for post, info in member['posts'].items():
            date = Utils.my_date_to_universal(info[0])
            if date_from <= date < date_to and info[1] != '!':
                made_counter += 1
        print()
        print(member['name'], 'made', made_counter, 'posts since', date_from.date())
        print(member['name'], 'has', counter, 'released posts since', date_from.date())
        print(member['name'] + "'s", 'average results are:')
        for stat_type, values in stats.items():
            summ = 0
            for value in values:
                summ += value
            print(stat_type + ':', summ / counter)

    def analyze_rubrics(self, regBook):
        if not self.init_vk():
            return
        print('\nInput date to collect stats from in format YYYY.MM.DD\n')
        date_from = Utils.my_date_to_universal(input())
        print()
        print('Input date to collect stats to in format YYYY.MM.DD\n')
        date_to = Utils.my_date_to_universal(input())
        print()
        print('\nInput rubric\n')
        rubric = input()
        print()
        news_posts = {}
        if rubric == 'News':
            member_amount = regBook.get_members_amount()
            for i in range(member_amount):
                member = Utils.parse_member_data(regBook.get_member_data(i))
                if member['rubric'] == rubric:
                    news_posts.update(member['posts'])
        rubric = Utils.rubric_to_hashtag(rubric)
        stats = defaultdict(list)
        counter = 0
        offset = -100
        date = datetime.now()
        while date >= date_from:
            offset += 100
            posts = self.vk.get_posts(offset, 100)
            for post in posts['items']:
                date = Utils.vk_date_to_universal(post['date'])
                if date >= date_to:
                    continue
                if date < date_from:
                    continue
                hashtag = Utils.get_hashtag(post).replace(' ', '')
                if hashtag == rubric or Utils.post_id_to_cached_link(post['id']) in news_posts\
                or 'postponed_id' in post and Utils.post_id_to_cached_link(post['postponed_id']) in news_posts:
                    votes = -1
                    if 'attachments' in post and 'poll' in post['attachments'][0]:
                        votes = self.vk.get_poll(post['attachments'][0]['poll']['id'])['votes']
                    print(date, Utils.post_id_to_link(post['id']), post['likes']['count'], post['reposts']['count'], votes)
                    for stat_type in ['comments', 'likes', 'reposts', 'views']:
                        stats[stat_type].append(post[stat_type]['count'])
                    stats['efficiency'].append((post['likes']['count']) / (post['views']['count']))
                    stats['votes'].append(votes)
                    counter += 1
        for stat_type, values in stats.items():
            summ = 0
            for value in values:
                summ += value
            print(stat_type + ':', summ / counter)

    @staticmethod
    def get_member_plot(regBook):
        member_data = Utils.choose_member(regBook, info=True)
        if not member_data:
            return
        days, amounts = Utils.get_member_posts_dates(member_data)
        new_days, new_amounts = Utils.regularize_data_for_plot(days, days, amounts, 10)
        pyplot.xlabel('Dates')
        pyplot.ylabel('Posts')
        pyplot.grid(True)
        pyplot.plot(new_days, new_amounts, 'g', linewidth='1')
        pyplot.plot(new_days, new_amounts, 'bx', markersize='7')
        days_to_show, dates_to_show = Utils.days_list_to_plot_ticks(new_days)
        pyplot.xticks(days_to_show, dates_to_show)
        pyplot.yticks(new_amounts, new_amounts)
        pyplot.show()

    @staticmethod
    def get_members_plot(regBook):
        member_amount = regBook.get_members_amount()
        pyplot.xlabel('Dates')
        pyplot.ylabel('Posts')
        pyplot.grid(True)
        all_days, all_amounts = [], []
        members_posts = {}
        member_days = {}
        for i in range(member_amount):
            member_data = regBook.get_member_data(i)
            days, amounts = Utils.get_member_posts_dates(member_data)
            member = Utils.parse_member_data(member_data)
            all_days += days
            members_posts[member['name']] = amounts
            member_days[member['name']] = days
        all_days = sorted(all_days)
        saved_new_days = []
        i = 0
        for member, posts in members_posts.items():
            new_days, new_amounts = Utils.regularize_data_for_plot(all_days, member_days[member], posts, 10)
            all_amounts += new_amounts
            if len(new_days) > len(saved_new_days):
                saved_new_days = new_days
            pyplot.plot(new_days, new_amounts, marker='x', linewidth=0.5, markersize='7', label=member)
            i += 1
        days_to_show, dates_to_show = Utils.days_list_to_plot_ticks(saved_new_days)
        pyplot.xticks(days_to_show, dates_to_show)
        pyplot.yticks(all_amounts, all_amounts)
        pyplot.legend()
        pyplot.show()

    def get_postponed_link_for_post(self, _):
        if not self.init_vk():
            return
        print('\nInput your link\n')
        posts = self.vk.get_post(Utils.clear_string(input()).split('_')[-1])
        if len(posts) == 0:
            print('\nWrong link')
            return
        if len(posts) > 1:
            print('\nSomething went wrong, you got a lot of posts:', len(posts))
            return
        post = posts[0]
        if 'postponed_id' in post:
            print()
            print(Utils.post_id_to_cached_link(post['postponed_id']))
        else:
            print('\nThis post was not postponed!')

    def get_actual_link_for_post(self, _):
        if not self.init_vk():
            return
        print('\nInput your link\n')
        postponed_id = int(Utils.clear_string(input()).split('_')[-1])
        print('\nInput date to search back to in format YYYY.MM.DD\n')
        date_from = Utils.my_date_to_universal(input())
        print()
        offset = -100
        date = datetime.now()
        while date >= date_from:
            offset += 100
            posts = self.vk.get_posts(offset, 100)
            for post in posts['items']:
                date = Utils.vk_date_to_universal(post['date'])
                if date < date_from:
                    continue
                if 'postponed_id' in post and post['postponed_id'] == postponed_id:
                    print(Utils.post_id_to_link(post['id']))
                    return
        print('Post not found in time period since', date_from.strftime('%Y.%m.%d'))
