import vk

from time import sleep


class Vk:
    def __init__(self, command_args):
        print('\nInitializing vk api')
        self.api_version = '5.101'
        self.community_id = '17592208'
        self.admins = [5962550, 155003349, 151414919, 295683611, 328873069]
        self.user_api = None
        self.community_api = None
        if command_args.login and command_args.password:
            self.user_api = \
                vk.API(vk.AuthSession('3116505', command_args.login, command_args.password, scope='wall, groups'))
        else:
            print('Not enough data to initialize vk user api')
        if command_args.token:
            self.community_api = vk.API(vk.Session(access_token=command_args.token))
        else:
            print('Not enough data to initialize vk community api')

    def get_conversations(self, offset=0, count=200):
        if not self.community_api:
            return None
        return self.community_api.messages.getConversations(group_id=self.community_id, v=self.api_version,
                                                            offset=offset, count=count, extended=1)

    def get_messages(self, conversation):
        if not self.community_api:
            return None
        peer_id = conversation['conversation']['peer']['id']
        return self.community_api.messages.getHistory(peer_id=peer_id, v=self.api_version)

    def print_conversations(self, user_id):
        conversations = self.get_conversations()
        for item in conversations['items']:
            messages = self.get_messages(item)
            for message in messages['items']:
                if 'admin_author_id' in message and message['admin_author_id'] == user_id:
                    print(message)

    def get_poll(self, poll_id):
        if not self.user_api:
            return None
        owner_id = '-' + self.community_id
        try:
            return self.user_api.polls.getById(owner_id=owner_id, poll_id=poll_id, v=self.api_version)
        except:
            sleep(0.5)
            return self.get_poll(poll_id)


    def get_posts(self, offset=0, count=100, type_filter='all', owner_id=None):
        if not self.user_api:
            return None
        if not owner_id:
            owner_id = '-' + self.community_id
        try:
            return self.user_api.wall.get(owner_id=owner_id, v=self.api_version, 
                                          offset=offset, count=count, filter=type_filter)
        except:
            sleep(0.5)
            return self.get_posts(offset, count, type_filter, owner_id)

    def get_post(self, post_id):
        if not self.user_api:
            return None
        wall_post_id = '-' + self.community_id + '_' + str(post_id)
        try:
            return self.user_api.wall.getById(posts=wall_post_id, v=self.api_version)
        except:
            sleep(0.5)
            return self.get_post(post_id)

    def get_reposts(self, item_id):
        if not self.user_api:
            return None
        try:
            owner_id = '-' + self.community_id
            return self.user_api.likes.getList(type='post', owner_id=owner_id,
                                               item_id=item_id, v=self.api_version, filter='copies')
        except:
            sleep(0.5)
            return self.get_reposts(item_id)

    def get_post_info(self, offset=0, amount=1, type_filter='all', i=0):
        if not self.user_api:
            return None
        posts = self.get_posts(offset, amount, type_filter)
        for key, value in posts['items'][i].items():
            print(key, value)
