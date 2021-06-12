using System;
using System.Threading;

namespace Probability
{
    public class ActionSyncRetry
    {
        public static void Run(Func<bool> action, int maxRetryTimes = 0, 
            String actionName = "Action", int waitTimeout = 1000)
        {
            if (maxRetryTimes == 0) maxRetryTimes = Int32.MaxValue;
            bool isSucceeded = false;
            
            for (int retry = 1; !isSucceeded && retry - 1 <= maxRetryTimes; retry++) {
                try
                {
                    isSucceeded = action.Invoke();
                }
                catch (Exception exception)
                {
                    Console.WriteLine($"Exception: Unable to {actionName}");
                    Console.WriteLine(exception.Message);
                    if (retry <= maxRetryTimes)
                    {
                        Thread.Sleep(waitTimeout);
                        Console.WriteLine($"Info: Retrying {actionName} ... {retry}");
                    }
                    else
                    {
                        throw new Exception($"Exception: Unable to {actionName}");
                    }
                }
            }
        }
    }
}