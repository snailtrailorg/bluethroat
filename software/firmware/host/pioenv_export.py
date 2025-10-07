Import("env")

# Set environment variables for Kconfig to identify the target device
env['ENV']['BLUETHROAD_PIOENV'] = env['PIOENV']
#print(env.Dump())

## Another method use os.environ also feasible
#import os
#os.environ['BLUETHROAD_PIOENV'] = env['PIOENV']
#print(os.environ)