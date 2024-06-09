from setuptools import setup, find_packages

# Read the content of the README file
with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

setup(
    name="rg_gateway",
    version="0.1.0",
    author="Jan Mrázek",
    author_email="email@honzamrazek.cz",
    description="A gateway for routing game boxes",
    long_description=long_description,
    long_description_content_type="text/markdown",
    packages=find_packages(),
    include_package_data=True,
    install_requires=[
        "Flask>=3.0.0",
        "pyserial>=3.5",
        "click>=7.0"
    ],
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
        "Framework :: Flask",
        "Topic :: Software Development :: Embedded Systems"
    ],
    python_requires='>=3.6',
    entry_points={
        'console_scripts': [
            'your_flask_serial_app=your_flask_serial_app.cli:main',
        ],
    },
)
