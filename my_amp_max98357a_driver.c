#include <linux/module.h> //커널 모듈 
#include <linux/kernel.h> // 커널로그 사용하기 위해 
#include <linux/init.h> // 모듈 시작 종료 매크로 
#include <linux/platform_device.h> // 플랫폼 드라이버 구조체 사용
#include <linux/gpio/consumer.h> // 최신 GPIO 제어함수들 gpiod
#include <linux/of.h> // Device Tree 파싱용 


struct my_amp_data {
    struct gpio_desc *sdmode_gpio;
};

static int my_amp_probe(struct platform_device *pdev)
{
    struct my_amp_data *data;
    struct device *dev = &pdev->dev;

    pr_info("My-amp-driver : 장치 드라이버 초기화 시작\n");
    //메모리 할당
    struct my_amp_data *data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
    if(!data)
        return -ENOMEM;

    //gpio 가져오기 
    data->sdmode_gpio = devm_gpiod_get(&pdev->dev, "sdmode", GPIOD_OUT_LOW);

    if (IS_ERR(data->sdmode_gpio)) {
        pr_err("My-amp-driver: GPIO sdmode를 찾을수 없음 DTS를 확인\n");
        return PTR_ERR(data->sdmode_gpio);
    }
    // 앰프 켜기 
    pr_info("My-amp-driver: 앰프 스위치를 킨다 GPIO->High \n");
    gpiod_set_value(data->sdmode_gpio, 1);

    // 데이터 맡기기? 
    platform_set_drvdata(pdev, data);

    pr_info("My-amp-driver: 초기화 완료 \n");
    return 0;
}

static int my_amp_remove(struct platform_device *pdev)
{
    struct my_amp_data *data = platform_get_drvdata(pdev);
    pr_info(m="My-amp-driver: 드라이버 종료\n");

    if(data->sdmode_gpio){
        gpiod_set_value(data->sdmode_gpio, 0);
        pr_info("My-amp-driver:앰프 스위치 off sdmode 0v\n");
    }

    return 0;
}

//매칭테이블 dts 파일과 일치해야함 
static const struct of_device_id my_amp_dt_ids[] = {
    { .compatible = "test,my-amp_max98357a"},
    { }
};
MODULE_DEVICE_TABLE(of, my_amp_dt_ids);

static struct platform_driver my_amp_driver = {
    .prove = my_amp_probe,
    .remove = my_amp_remove,
    .driver = {
        .name = "my_amp_max98357_driver",
        .of_match__table = my_amp_dt_ids,
    },
};
module_platform_driver(my_amp_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("natsuki");
MODULE_DESCRIPTION("custom driver for max98357a on raspverry pi 5);


