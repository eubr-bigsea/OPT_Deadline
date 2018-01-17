# coding:utf-8
import cherrypy
import subprocess
import os
from jinja2 import Environment, FileSystemLoader
import zipfile
import tarfile
import time
import datetime
import random
import string
import glob
import re
import json
import shutil
import math
from distutils.dir_util import copy_tree
from concurrent.futures import ProcessPoolExecutor

env = Environment(loader=FileSystemLoader('templates'))

PATH = os.path.abspath(os.path.dirname(__file__))


def future_done_callback(fut):
    print("\n\n\nDONE\n\n\n\n")
    try:
        with open(os.path.join(SETTINGS.TMP_FOLDER, fut.application_session_id, 'completed.txt'), 'w') as f:
            f.write(str(time.time()))
    except:
        print("Could not create completed output file")

class Settings(object):
    APP_FILES_DEFAULT_FOLDER = '/opt/OPT_DEADLINE_WS/app_files'
    TMP_DEFAULT_FOLDER = '/opt/OPT_DEADLINE_WS/tmp'
    DAGSIM_DEFAULT_PATH = '/opt/dagSim'
    OPT_IC_BINARY_DEFAULT_PATH = '/opt/OPT_IC/src/opt_ic'
    CONFIGURATION_DEFAULT_FOLDER = '/opt/OPT_DEADLINE_WS/configurations'
    OPT_DEADLINE_DEFAULT_PATH = '/opt/OPT_DEADLINE'

    def __init__(self):
        D = {}
        # Load settings from file
        try:
            with open('settings.txt', 'r') as f:
                D = json.loads(f.read())
        except:
            # Set default settings
            D['APP_FILES_FOLDER'] = os.path.abspath(Settings.APP_FILES_DEFAULT_FOLDER)
            D['TMP_FOLDER'] = os.path.abspath(Settings.TMP_DEFAULT_FOLDER)
            D['DAGSIM_PATH'] = os.path.abspath(Settings.DAGSIM_DEFAULT_PATH)
            D['OPT_IC_BINARY_PATH'] = os.path.abspath(Settings.OPT_IC_BINARY_DEFAULT_PATH)
            D['CONFIGURATION_FOLDER'] = os.path.abspath(Settings.CONFIGURATION_DEFAULT_FOLDER)
            D['OPT_DEADLINE_PATH'] = os.path.abspath(Settings.OPT_DEADLINE_DEFAULT_PATH)

        super().__setattr__('D', D)

    def __getattr__(self, attr):
        return self.D[attr]

    def __setattr__(self, attr, value):
        self.D[attr] = value

    def save(self):
        with open('settings.txt', 'w') as f:
            f.write(json.dumps(self.D, indent=4))

SETTINGS = Settings()

class Configuration(object):
    def __init__(self, configuration_name, applications):
        self.configuration_name = configuration_name
        self.applications = applications

    def save(self, path=None):
        if path is None:
            path = os.path.join(SETTINGS.CONFIGURATION_FOLDER, self.configuration_name + '.txt')
        with open(path, 'w') as f:
            f.write('# {}\n'.format(self.configuration_name))
            for app in self.applications:
                f.write(' '.join(app) + '\n')

    @staticmethod
    def load(configuration_name):
        path = os.path.join(SETTINGS.CONFIGURATION_FOLDER, configuration_name + '.txt')

        return Configuration.load_from_path(path)

    @staticmethod
    def load_from_path(path):
        if os.path.exists(path):
            with open(path, 'r') as f:
                lines = f.readlines()
                configuration_name = list(filter(lambda x : x.startswith('#'), lines))[-1][1:].strip()
                applications = list(map(lambda x : list(map(lambda y : y.strip(), x.split(' '))), filter(lambda x : len(x.strip()) > 0 and not x.startswith('#'), lines)))

                return Configuration(configuration_name, applications)
        else:
            raise Exception('Configuration not found: {}'.format(path))

    @staticmethod
    def list_configurations():
        return list(map(lambda x : os.path.basename(x)[:-4], glob.glob(os.path.join(SETTINGS.CONFIGURATION_FOLDER, '*'))))


def generate_id():
    return ''.join(random.choice(string.ascii_letters + string.digits) for _ in range(10))


class ConfigurationHandler(object):
    @cherrypy.expose
    @cherrypy.tools.json_in()
    @cherrypy.tools.json_out()
    def save(self):
        configuration_json = cherrypy.request.json

        configuration_name = configuration_json['configuration_name'].strip()
        applications = list(map(lambda x: list(map(lambda y: y.strip(), x)), configuration_json['applications']))

        configuration = Configuration(configuration_name, applications)
        configuration.save()


        status = 200
        message = 'Configuration saved'
        return {'status' : status, 'message' : message}

class ExecutorService(object):
    def __init__(self):
        self.running_jobs = []


    def start_job(self, configuration, algorithms, deadline):
        ppe = ProcessPoolExecutor(max_workers=1)
        application_session_id = 'run_' + configuration.configuration_name.replace(' ', '_') + '_' + generate_id()
        fut = ppe.submit(ExecutorService._start_job, configuration, algorithms, application_session_id, deadline)
        fut.application_session_id = application_session_id

        fut.add_done_callback(future_done_callback)
        ppe.shutdown(wait=False)

    @staticmethod
    def _start_job(configuration, algorithms, application_session_id, deadline):
        path = os.path.join(SETTINGS.TMP_FOLDER, application_session_id)
        # create folder
        os.mkdir(path)
        os.mkdir(os.path.join(path, 'tmp'))
        os.mkdir(os.path.join(path, 'output'))

        process_path = os.path.join(path, 'process.txt')
        config_path = os.path.join(path, 'config.txt')
        output_path = os.path.join(path, 'output')

        with open(os.path.join(path, 'started.txt'), 'w') as f:
            f.write(str(time.time()))

        with open(os.path.join(path, 'deadline.txt'), 'w') as f:
            f.write(str(deadline))

        configuration.save(process_path)

        with open(config_path, 'w') as f:
            f.write(os.path.abspath(SETTINGS.APP_FILES_FOLDER) + '\n')
            f.write(os.path.abspath(SETTINGS.DAGSIM_PATH) + '\n')
            f.write(os.path.abspath(SETTINGS.APP_FILES_FOLDER) + '\n')
            f.write(os.path.abspath(SETTINGS.OPT_IC_BINARY_PATH) + '\n')
            f.write(os.path.abspath(os.path.join(path, 'tmp')))


        def run_application_algorithm(algorithm_format):
            args = [os.path.join(SETTINGS.OPT_DEADLINE_PATH, 'opt_deadline'), process_path, config_path, deadline, algorithm_format]
            print("calling: {}".format(' '.join(args)))

            stdout_file = open(os.path.join(output_path, 'algorithm' + algorithm_format + '_out.txt'), 'w')
            stderr_file = open(os.path.join(output_path, 'algorithm' + algorithm_format + '_err.txt'), 'w')

            subprocess.run(args, stdout=stdout_file, stderr=stderr_file, cwd=output_path)

            stdout_file.close()
            stderr_file.close()

        if algorithms['algorithm1'] == True:
            run_application_algorithm('-1')
        if algorithms['algorithm2'] == True:
            run_application_algorithm('-2')




executor_service = ExecutorService()

def read_file_content(path):
    try:
        with open(path, 'r') as f:
            content = f.read()
        return content if len(content) > 0 else '-'
    except:
        return "File not found"

def parse_query_name(path):
    content = read_file_content(path)

    line = list(filter(lambda x : not x.startswith('#') and len(x.strip()) > 0, content.split('\n')))[-1]

    return line.split(' ')[0].strip()

def parse_result_ncore(path):
    content = read_file_content(path)
    index = content.rfind('----DUMP PROCESS----')
    ncores = re.findall('No. Cores: (\d+);', content[index:])
    return list(map(int, ncores))

def parse_result_computed_deadlines(path):
    content = read_file_content(path)
    index = content.rfind('----DUMP PROCESS----')
    deadlines = re.findall('Deadline: ([0-9\.e\+]+)\n', content[index:])
    return list(map(float, deadlines))

class ApplicationStatus(object):
    def __init__(self, application_session_id):
        self.application_session_id = application_session_id
        self.name = '_'.join(self.application_session_id.split('_')[1:-1])

        try:
            self.output_files = glob.glob(os.path.join(SETTINGS.TMP_FOLDER, application_session_id, 'output', '*.txt'))

            try:
                self.configuration = Configuration.load_from_path(os.path.join(SETTINGS.TMP_FOLDER, application_session_id, 'process.txt'))
            except:
                self.configuration = Configuration('', [[]])

            if os.path.exists(os.path.join(SETTINGS.TMP_FOLDER, application_session_id, 'completed.txt')):
                self.status = 'COMPLETED'
                self.completed_time = read_file_content(os.path.join(SETTINGS.TMP_FOLDER, application_session_id, 'completed.txt'))
                self.completed_time_format = datetime.datetime.fromtimestamp(float(self.completed_time)).strftime('%Y-%m-%d at %H:%M:%S')
                self.computed_deadline = read_file_content(os.path.join(SETTINGS.TMP_FOLDER, application_session_id, 'output', 'result.txt'))

                self.total_cost = 0.0
                result_filename = glob.glob(os.path.join(SETTINGS.TMP_FOLDER, application_session_id, 'output', 'output_result_Algorithm2*'))[0]
                self.n_cores = parse_result_ncore(os.path.join(SETTINGS.TMP_FOLDER, application_session_id, 'output', result_filename))
                self.deadlines = parse_result_computed_deadlines(os.path.join(SETTINGS.TMP_FOLDER, application_session_id, 'output', result_filename))
                for nc, w in zip(self.n_cores, self.configuration.applications):
                    self.total_cost += int(nc) * float(w[-1])

                self.total_cost = str(self.total_cost)
            else:
                self.status = 'RUNNING'
                self.computed_deadline = '-'
                self.completed_time = '-'
                self.completed_time_format = '-'
                self.total_cost = '-'
                self.n_cores = ['0']*len(self.configuration.applications)
                self.deadlines = ['-']*len(self.configuration.applications)

            try:
                self.started_time = read_file_content(os.path.join(SETTINGS.TMP_FOLDER, application_session_id, 'started.txt'))
                self.started_time_format = datetime.datetime.fromtimestamp(float(self.started_time)).strftime('%Y-%m-%d at %H:%M:%S')
            except:
                self.started_time = '-'
                self.started_time_format = '-'

            try:
                self.initial_deadline = read_file_content(os.path.join(SETTINGS.TMP_FOLDER, application_session_id, 'deadline.txt'))
            except:
                self.initial_deadline = '-'

            self.V = []
            self.num_vms = []
            self.query_name = []
            for i,app in enumerate(self.configuration.applications):
                try:
                    config_app_path = os.path.abspath(os.path.join(SETTINGS.APP_FILES_FOLDER, app[-2]))
                    content = read_file_content(config_app_path)
                    line = list(filter(lambda x : not x.startswith('#') and len(x.strip()) > 0, content.split('\n')))[-1]
                    self.V.append(line.split(' ')[-2])
                    self.num_vms.append(math.ceil(int(self.n_cores[i]) / int(self.V[i])))
                    self.query_name.append(parse_query_name(config_app_path))
                except Exception as e:
                    print(e)
                    self.V.append('1')
                    self.num_vms.append(self.n_cores[i])
                    self.query_name.append('-')

            self.total_cores = sum(map(int, self.n_cores))
            self.total_vms = sum(map(int, self.num_vms))

        except Exception as e:
            print("Exception found in ApplicationStatus __init__: {}".format(e))
            self.output_files = []
            self.status = 'ERROR'
            self.started_time = '-'

    @staticmethod
    def get_all_application_statuses():
        app_session_ids = list(map(os.path.basename, glob.glob(os.path.join(SETTINGS.TMP_FOLDER, 'run_*'))))
        return sorted([ApplicationStatus(app_session_id) for app_session_id in app_session_ids], key=lambda x : x.started_time, reverse=True)


class OPT_DEADLINE_WS(object):
    @cherrypy.expose
    def index(self, **kwargs):
        tmpl = env.get_template('index.html')

        application_files = sorted(map(os.path.basename, glob.glob(os.path.join(SETTINGS.APP_FILES_FOLDER, 'app_*.csv'))))
        jobs_files = sorted(map(os.path.basename, glob.glob(os.path.join(SETTINGS.APP_FILES_FOLDER, 'jobs_*.csv'))))
        stages_files = sorted(map(os.path.basename, glob.glob(os.path.join(SETTINGS.APP_FILES_FOLDER, 'stages_*.csv'))))
        tasks_files = sorted(map(os.path.basename, glob.glob(os.path.join(SETTINGS.APP_FILES_FOLDER, 'tasks_*.csv'))))
        lua_files = sorted(map(os.path.basename, glob.glob(os.path.join(SETTINGS.APP_FILES_FOLDER, '*.lua'))))
        config_app_files = sorted(map(os.path.basename, glob.glob(os.path.join(SETTINGS.APP_FILES_FOLDER, 'ConfigApp_*.txt'))))

        all_files = [application_files, jobs_files, stages_files, tasks_files, lua_files, config_app_files]

        kwargs['files'] = all_files
        kwargs['configurations'] = Configuration.list_configurations()
        #kwargs['configuration'] = Configuration('conf1', [['app_BULMA.csv', 'jobs_BULMA.csv', 'stages_BULMA.csv', 'tasks_BULMA.csv', 'test_BULMA.lua', 'ConfigApp_BULMA.txt', '1'], ['app_D.csv', 'jobs_D.csv', 'stages_D.csv', 'tasks_D.csv', 'test_D.lua', 'ConfigApp_D.txt', '4']])
        try:
            kwargs['configuration'] = Configuration.load(kwargs['conf'])
        except:
            kwargs['configuration'] = Configuration('New configuration', [['-']*6 + ['1']])
        return tmpl.render(params=kwargs)

    @cherrypy.expose
    def results(self):
        app_status = ApplicationStatus.get_all_application_statuses()
        tmpl = env.get_template('results.html')
        return tmpl.render(params=app_status)

    @cherrypy.expose
    def view_results(self, app_session_id):
        app_status = ApplicationStatus(app_session_id)
        output_params = []
        for output_file in app_status.output_files:
            file_content = read_file_content(output_file)
            output_params.append({'filename' : os.path.basename(output_file), 'content' : file_content, 'num_rows' : min(len(file_content.split('\n')), 50)})

        params = {'output_params' : output_params, 'app_session_id' : app_session_id, 'app_status' : app_status}
        tmpl = env.get_template('view_results.html')
        return tmpl.render(params=params)

    @cherrypy.expose
    def settings(self):
        tmpl = env.get_template('settings.html')
        return tmpl.render(params=SETTINGS)

    @cherrypy.expose
    @cherrypy.tools.json_in()
    @cherrypy.tools.json_out()
    def savesettings(self):
        settings_json = cherrypy.request.json
        for k,v in settings_json.items():
            SETTINGS.__setattr__(k, v)

        SETTINGS.save()

        return {'status':200, 'message': "settings updated"}

    @cherrypy.expose
    def upload(self, myFile):
        tmp_file = 'import_' + generate_id()
        archive_folder = os.path.join(SETTINGS.TMP_FOLDER, tmp_file)
        os.mkdir(archive_folder)

        archive_location = os.path.join(archive_folder, 'archive')

        # Save the archive
        with open(archive_location, 'wb') as f:
            f.write(myFile.file.read())

        # Extract the archive
        try:
            with zipfile.ZipFile(open(archive_location, 'rb'), 'r') as f:
                f.extractall(archive_folder)

        except zipfile.BadZipFile:
            with tarfile.open(archive_location) as f:
                f.extractall(archive_folder)


        # Select the files to extract
        all_lua_files = sorted(glob.glob(os.path.join(archive_folder, '**/logs/*/*.lua'), recursive=True))

        def get_query(p):
            return os.path.basename(os.path.abspath(os.path.join(p, *(['..']*3))))
        def get_core_folder(p):
            return os.path.abspath(os.path.join(p, *(['..']*4)))

        core_folders = sorted(set(map(get_core_folder, all_lua_files)), key=lambda x : int(os.path.basename(x).split('_')[0]))
        queries = sorted(set(map(get_query, all_lua_files)))

        # Select the core folder
        FOLDER = core_folders[len(core_folders)//2]
        def choose_folder(paths):
            return sorted(paths)[0]

        selected_lua_files = []
        for q in queries:
            try:
                selected_lua_files.append(choose_folder(glob.glob(os.path.join(FOLDER, q, 'logs/*/*.lua'))))
            except Exception as e:
                print('\n\nQUERY {} not found in path {}\n\n'.format(q, os.path.join(FOLDER, q, 'logs/*/*.lua')))


        # Postprocess the lua file

        for luafilename in selected_lua_files:
            # Copy the content of the directory
            query_name = get_query(luafilename)
            lua_directory = os.path.abspath(os.path.join(luafilename, '..'))

            app_path = glob.glob(os.path.join(lua_directory, 'app_*'))[0]
            shutil.copy2(app_path, os.path.join(SETTINGS.APP_FILES_FOLDER, 'app_' + query_name + '.csv'))

            jobs_path = glob.glob(os.path.join(lua_directory, 'jobs_*'))[0]
            shutil.copy2(jobs_path, os.path.join(SETTINGS.APP_FILES_FOLDER, 'jobs_' + query_name + '.csv'))

            stages_path = glob.glob(os.path.join(lua_directory, 'stages_*'))[0]
            shutil.copy2(stages_path, os.path.join(SETTINGS.APP_FILES_FOLDER, 'stages_' + query_name + '.csv'))

            tasks_path = glob.glob(os.path.join(lua_directory, 'tasks_*'))[0]
            shutil.copy2(tasks_path, os.path.join(SETTINGS.APP_FILES_FOLDER, 'tasks_' + query_name + '.csv'))
            try:
                config_app_path = glob.glob(os.path.join(lua_directory, 'ConfigApp_*'))[0]
                shutil.copy2(config_app_path, os.path.join(SETTINGS.APP_FILES_FOLDER, 'ConfigApp_' + query_name + '.txt'))
            except:
                print ("Could not found ConfigApp for query {}".format(query_name))

            def convert_path(query_name):
                def wrapped(match):
                    path = match.group(1)
                    span = match.span(1)

                    return match.string[match.span()[0]:span[0]] + os.path.abspath(os.path.join(SETTINGS.APP_FILES_FOLDER, 'test_' + query_name, os.path.basename(path))) + match.string[span[1]:match.span()[1]]

                return wrapped

            with open(luafilename, 'r') as f:
                filecontent = f.read()
            # filecontent = re.sub(r'solver\.fileToArray\("[^"]+"\)', 'solver.fileToArray("' + os.path.abspath(os.path.join(SETTINGS.APP_FILES_FOLDER, 'test_' + query_name)) + '")', filecontent)
            filecontent = re.sub(r'solver\.fileToArray\("([^"]+)"\)', convert_path(query_name), filecontent)
            filecontent = re.sub(r'Nodes = \d+', 'Nodes = @@nodes@@', filecontent)


            with open(luafilename, 'w') as f:
                f.write(filecontent)

            shutil.copy2(luafilename, os.path.join(SETTINGS.APP_FILES_FOLDER, 'test_' + query_name + '.lua'))
            try:
                os.mkdir(os.path.abspath(os.path.join(SETTINGS.APP_FILES_FOLDER, 'test_' + query_name)))
            except:
                # Folder already exists, files will be overwritten
                pass
            for f in glob.glob(os.path.join(lua_directory, 'S*.txt')):
                shutil.copy2(f, os.path.abspath(os.path.join(SETTINGS.APP_FILES_FOLDER, 'test_' + query_name)))

        raise cherrypy.InternalRedirect('/', 'message=Upload ok')

    @cherrypy.expose
    @cherrypy.tools.json_in()
    @cherrypy.tools.json_out()
    def run_application(self):
        configuration_json = cherrypy.request.json

        configuration_name = configuration_json['configuration_name'].strip()
        algorithms = configuration_json['algorithms']
        deadline = configuration_json['deadline']

        configuration = Configuration.load(configuration_name)
        executor_service.start_job(configuration, algorithms, deadline)

        return 'Job submitted'



if __name__ == '__main__':
    conf = {
        '/': {
            'tools.sessions.on': True,
            'tools.staticdir.root': PATH
        },
        '/static': {
            'tools.staticdir.on': True,
            'tools.staticdir.dir': 'static'
        }
    }

    web_service = OPT_DEADLINE_WS()
    web_service.config = ConfigurationHandler()
    cherrypy.server.socket_host = '0.0.0.0'
    cherrypy.quickstart(web_service, '/', conf)
