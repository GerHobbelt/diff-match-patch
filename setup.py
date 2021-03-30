# -*- coding: utf-8 -*-

try:
    from pip._internal.req import parse_requirements
except ImportError:
    from pip.req import parse_requirements

from setuptools import setup, find_packages

install_requires = [str(ir.req) for ir in parse_requirements('requirements.txt', session=False)]

setup(
    name='diff-match-patch',
    version='0.1',
    description='diff-match-patch',
    long_description=open('README.md').read().strip(),
    classifiers=[
        'Development Status :: 0.1 - Stable',
        'Environment :: Plugins',
        'Intended Audience :: Developers',
        'License :: Other/Proprietary License',
        'Operating System :: OS Independent',
        'Programming Language :: Python',
        'Topic :: Software Development :: Libraries :: Python Modules',
    ],
    keywords='diff-match-patch',
    author='Sidiropulo Eduard',
    author_email='e.sidiropulo@aviata.me',
    url='https://github.com/sidiropulos/diff-match-patch.git',
    license='Other/Proprietary License',
    install_requires=install_requires,
    packages=find_packages(),
    zip_safe=False,
)
