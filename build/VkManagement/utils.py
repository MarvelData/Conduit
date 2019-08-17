import sys

from collections import defaultdict
from datetime import datetime
from datetime import timedelta


class Utils:
    @staticmethod
    def my_date_to_universal(date):
        return datetime.strptime(date, '%Y.%m.%d')

    @staticmethod
    def vk_date_to_universal(date):
        return datetime.fromtimestamp(date)

    @staticmethod
    def post_id_to_link(post_id, prefix='https://vk.com/wall-17592208_'):
        return prefix + str(post_id)

    @staticmethod
    def post_id_to_cached_link(post_id):
        return 'https://vk.com/mu_marvel?w=wall-17592208_' + str(post_id)

    @staticmethod
    def get_hashtag(post):
        hashtag = post['text'].split('\n')[0]
        if '#' in hashtag:
            return hashtag
        else:
            return '#' + hashtag

    @staticmethod
    def rubric_to_hashtag(rubric):
        return '#MU_' + rubric

    @staticmethod
    def clear_string(string):
        return string.replace(' ', '').replace('\n', '').replace('\r', '')

    @staticmethod
    def list_to_string(my_list):
        string = ''
        for elem in my_list:
            string += elem + ', '
        return string[:-2]

    @staticmethod
    def days_list_to_plot_ticks(days):
        days_to_show, dates_to_show = [], []
        last_shown_day = -sys.maxsize
        for i, day_number in enumerate(days):
            if day_number - last_shown_day > (days[-1] - days[0]) / 9:
                last_shown_day = day_number
                days_to_show.append(days[i])
                dates_to_show.append((Utils.my_date_to_universal('2019.01.01') + timedelta(days=days[i])).date())
        return days_to_show, dates_to_show

    @staticmethod
    def regularize_data_for_plot(all_days, days, amounts, delta):
        new_days = [all_days[0]]
        new_amounts = [0]
        for i, day in enumerate(days):
            if day >= new_days[-1] + delta:
                new_days.append(new_days[-1] + delta)
                new_amounts.append(0)
            if day < new_days[-1] + delta:
                new_amounts[len(new_days) - 1] += amounts[i]
        return new_days, new_amounts

    @staticmethod
    def parse_member_data(data):
        member = {}
        posts = {}
        for line in data:
            if 'Short name:' in line:
                member['name'] = Utils.clear_string(line.split(':')[1])
            if 'Rubric:' in line:
                member['rubric'] = Utils.clear_string(line.split(':')[1])
            if len(line) > 5 and ' ' not in line:
                member['id'] = Utils.clear_string(line.split('\\')[0])
            if 'http' in line:
                columns = line.split(' ')
                amount = int(columns[1])
                for i in range(0, amount):
                    posts[Utils.clear_string(columns[2 + i * 2])] = \
                         [Utils.clear_string(columns[0]), Utils.clear_string(columns[3 + i * 2])]
        member['posts'] = posts
        return member

    @staticmethod
    def get_members_mapping(regBook):
        key_to_member = {}
        id_to_members = defaultdict(list)
        member_amount = regBook.get_members_amount()
        for i in range(member_amount):
            member = Utils.parse_member_data(regBook.get_member_data(i))
            key_to_member[member['id'] + Utils.rubric_to_hashtag(member['rubric'])] = member['name']
            id_to_members[member['id']].append(member['name'])
        return key_to_member, id_to_members

    @staticmethod
    def get_member_posts_dates(data):
        days, amounts = [], []
        date_from = Utils.my_date_to_universal('2019.01.01')
        for line in data:
            if 'http' in line:
                columns = line.split(' ')
                days.append((Utils.my_date_to_universal(Utils.clear_string(columns[0])) - date_from).days)
                amounts.append(int(Utils.clear_string(columns[1])))
        return days, amounts

    @staticmethod
    def choose_member(regBook, info=True, stats=False):
        members = regBook.get_members_list(info)
        extra_tools = None
        if stats:
            extra_tools = regBook.add_extra_tools(members, ['I would like to perform custom request'])
        for line in members:
            sys.stdout.write(line)
        while True:
            user_input = input()
            while not user_input:
                user_input = input()
            if extra_tools is not None and user_input in extra_tools:
                custom_data = []
                print('\nInput short name\n')
                custom_data.append('Short name: ' + input())
                print('\nInput rubric\n')
                custom_data.append('Rubric: ' + input())
                print('\nInput id\n')
                custom_data.append(input())
                return custom_data
            if user_input == '-1':
                return None
            member_data = regBook.get_member_data_safe(user_input)
            if not member_data:
                print('\nNo such element! Try again.')
                print('Input element name (u can also input according number)\n')
                print('Or input -1 to return =)')
            else:
                break
        for line in member_data[:-3]:
            sys.stdout.write(line)
        return member_data

    @staticmethod
    def filter_Kamil(member, post, engine):
        banned_list = [398504693, -134362257]
        fake_likes, fake_reposts, fake_views = 0, 0, 0
        if 'Kamil' in member['name']:
            reposters = engine.get_reposts(post['id'])
            for reposter in reposters['items']:
                if reposter in banned_list:
                    print(reposter)
                    reposter_posts = engine.get_posts(owner_id=reposter)
                    for reposter_post in reposter_posts['items']:
                        if 'copy_history' in reposter_post and reposter_post['copy_history'][0]['id'] == post['id']:
                            print(Utils.post_id_to_link(reposter_post['id'],
                                  prefix='https://vk.com/wall' + str(reposter) + '_'),
                                  reposter_post['likes']['count'], reposter_post['reposts']['count'])
                            fake_likes += reposter_post['likes']['count']
                            fake_reposts += reposter_post['reposts']['count']
                            fake_views += reposter_post['views']['count']
            print()
        return fake_likes, fake_reposts, fake_views
