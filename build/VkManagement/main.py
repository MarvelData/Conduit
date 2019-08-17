import argparse

from regbookConnector import RegBook


parser = argparse.ArgumentParser()

parser.add_argument('--login', default='')
parser.add_argument('--password', default='')
parser.add_argument('--token', default='')
parser.add_argument('--unix', default='')

command_args = parser.parse_args()


RegBook(command_args, silent=False)
