#include "Detector.h"
namespace Detect {
	auto ImgProcessor::thresholdBookmark(const cv::Mat& img) -> cv::Mat {
		//���ӶԱȶ�
		cv::Mat imgContrast, result;
		double alpha = 1.5; // �Աȶȿ���ϵ��
		int beta = 0; // ���ȿ���ֵ
		img.convertTo(imgContrast, -1, alpha, beta); // Ӧ�öԱȶȺ����ȵ���
		//��ֵ��
        cv::cvtColor(imgContrast, imgContrast, cv::COLOR_BGR2GRAY);
		cv::threshold(imgContrast, result, 100, 255, cv::THRESH_BINARY);
        // ���Ͳ���
        if (DILATE > 0) {
            cv::Mat dilateElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
            cv::dilate(result, result, dilateElement, cv::Point(-1, -1), DILATE);
        }

        // ��ʴ����
        if (ERODE > 0) {
            cv::Mat erodeElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
            cv::erode(result, result, erodeElement, cv::Point(-1, -1), ERODE);
        }
		return result;
	}
	auto ImgProcessor::insideprocess(const cv::Mat& img) -> std::vector<insidemarkpoint> {
		//inside��ָ�ڲ�����ǩ�У�����Ƕ�׵��������������ﷵ���������ĵ�
        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::Mat iimage = img;
        // ��������
        cv::findContours(iimage, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);

        std::vector<insidemarkpoint> insidePoints;

        for (int idx = 0; idx < hierarchy.size(); idx = hierarchy[idx][0])
        {
            if (hierarchy[idx][2] < 0 && hierarchy[idx][3] == -1) // ����û��������������
                continue;

            cv::RotatedRect rotatedRect = cv::minAreaRect(contours[idx]);

            // ȷ����ת���ε������Ԥ�ڷ�Χ��
            if (rotatedRect.size.area() < MIN_AREA || rotatedRect.size.area() > MAX_AREA) continue;


            cv::Point2f rectPoints[4];
            rotatedRect.points(rectPoints); // ��ȡ��ת���ε��ĸ��ǵ�

            // ȷ���ǵ����������
            std::sort(rectPoints, rectPoints + 4, [](const cv::Point2f& a, const cv::Point2f& b) {
                return a.x < b.x;
                });

            // �����������Ҳ�����ֱ�yֵ����ȷ��l1, l2�����Ϻ����£�r1, r2�����Ϻ�����
            std::sort(rectPoints, rectPoints + 2, [](const cv::Point2f& a, const cv::Point2f& b) {
                return a.y < b.y;
                });
            std::sort(rectPoints + 2, rectPoints + 4, [](const cv::Point2f& a, const cv::Point2f& b) {
                return a.y < b.y;
                });

            insidemarkpoint points = { rectPoints[0], rectPoints[1], rectPoints[2], rectPoints[3] };
            insidePoints.push_back(points);
        }

        // ���ݾ��ε����Ͻǵ����������
        std::sort(insidePoints.begin(), insidePoints.end(), [](const insidemarkpoint& a, const insidemarkpoint& b) {
            return a.l1.x < b.l1.x;
            });
        return insidePoints;
	}
    auto ImgProcessor::outsideprocess(const cv::Mat& img) -> std::vector<outsidemarkpoint> {
        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::Mat iimage = img.clone(); // Clone to avoid modifying the original image

        // ��������
        cv::findContours(iimage, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);

        std::vector<outsidemarkpoint> outsidePoints;

        for (int idx = 0; idx < hierarchy.size(); ++idx) {
            // �ҵ���û��������Ҳû�и�����������
            if (hierarchy[idx][2] == -1 && hierarchy[idx][3] == -1) {
                cv::RotatedRect rotatedRect = cv::minAreaRect(contours[idx]);

                // ��������������˲����ϴ�СҪ��ľ���
                if (rotatedRect.size.area() < MIN_AREA || rotatedRect.size.area() > MAX_AREA)
                    continue;

                cv::Point2f rectPoints[4];
                rotatedRect.points(rectPoints); // ��ȡ��ת���ε��ĸ��ǵ�

                // ����͵���rectPoints��ƥ��outsidemarkpoint�ṹ
                std::sort(rectPoints, rectPoints + 4, [](const cv::Point2f& a, const cv::Point2f& b) {
                    return a.x < b.x;
                    });

                // �����������Ҳ�����ֱ�yֵ����
                std::sort(rectPoints, rectPoints + 2, [](const cv::Point2f& a, const cv::Point2f& b) {
                    return a.y < b.y;
                    });
                std::sort(rectPoints + 2, rectPoints + 4, [](const cv::Point2f& a, const cv::Point2f& b) {
                    return a.y < b.y;
                    });

                // �������ĵ���ӵ����������
                outsidemarkpoint points = { rectPoints[0], rectPoints[1], rectPoints[2], rectPoints[3] };
                outsidePoints.push_back(points);
            }
        }

        // ���ݾ��ε����Ͻǵ����������
        std::sort(outsidePoints.begin(), outsidePoints.end(), [](const outsidemarkpoint& a, const outsidemarkpoint& b) {
            return a.l1.x < b.l1.x;
            });

        return outsidePoints;
    }
    void ImgProcessor::drawPoints(const cv::Mat& img,
        const std::vector<insidemarkpoint>& insidePoints,
        const std::vector<outsidemarkpoint>& outsidePoints,
        const std::string& winname) {
        cv::Mat frame= img.clone();
        // Ϊ�ⲿ�����
        for (const auto& point : outsidePoints) {
            // ʹ�ú�ɫ����ⲿ��
            drawPoint(frame, point, cv::Scalar(0, 0, 255));
        }
        // Ϊ�ڲ������
        for (const auto& point : insidePoints) {
            // ʹ����ɫ����ڲ���
            drawPoint(frame, point, cv::Scalar(0, 255, 0));
        }



        // ��ʾ���е�������ͼ��
        cv::imshow(winname, frame);
        cv::waitKey(1);
    }


    template <typename T>
    void ImgProcessor::drawPoint(cv::Mat& img, const T& point, const cv::Scalar& color) {
        // ����ÿ����
        cv::circle(img, point.l1, 5, color, -1); // ���Ͻǵ�
        cv::circle(img, point.l2, 5, color, -1); // ���½ǵ�
        cv::circle(img, point.r1, 5, color, -1); // ���Ͻǵ�
        cv::circle(img, point.r2, 5, color, -1); // ���½ǵ�

        // ��˳��������
        cv::line(img, point.l1, point.r1, color, 2);
        cv::line(img, point.r1, point.r2, color, 2);
        cv::line(img, point.r2, point.l2, color, 2);
        cv::line(img, point.l2, point.l1, color, 2);

        // ���ÿ���������
        std::ostringstream l1Text, l2Text, r1Text, r2Text;
        l1Text << "(" << point.l1.x << "," << point.l1.y << ")";
        l2Text << "(" << point.l2.x << "," << point.l2.y << ")";
        r1Text << "(" << point.r1.x << "," << point.r1.y << ")";
        r2Text << "(" << point.r2.x << "," << point.r2.y << ")";

        cv::putText(img, l1Text.str(), point.l1, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 2);
        cv::putText(img, l2Text.str(), point.l2, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 2);
        cv::putText(img, r1Text.str(), point.r1, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 2);
        cv::putText(img, r2Text.str(), point.r2, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 2);

        // �������ĵ�����
        cv::Point2f center = (point.l1 + point.r2) * 0.5f;
        std::ostringstream centerText;
        centerText << "Center: (" << center.x << "," << center.y << ")";


        // ���㲢��ʾ���εĴ�С����Ⱥ͸߶ȣ�
        float width = cv::norm(point.l1 - point.r1);
        float height = cv::norm(point.l1 - point.l2);
        std::ostringstream sizeText;
        sizeText << "Size: " << width << "x" << height << "=" << width * height;

        cv::circle(img, center, 5, cv::Scalar(0, 255, 255), -1); // ���½ǵ�
        cv::Point2f sizePos = center + cv::Point2f(20, 20); // ����С��Ϣλ����΢���ƣ����������ĵ������ص�

        cv::putText(img, centerText.str(), sizePos, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 255), 2); // ʹ�����ɫ������ĵ�����
        cv::putText(img, sizeText.str(), sizePos + cv::Point2f(0, 20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 255), 2); // ʹ�����ɫ������δ�С
   
    }
}
