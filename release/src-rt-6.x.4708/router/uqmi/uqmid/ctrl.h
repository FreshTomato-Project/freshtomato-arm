/*
 * uqmi -- tiny QMI support implementation
 *
 * Copyright (C) 2023 Alexander Couzens <lynxis@fe80.eu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef __UQMID_CTRL_H
#define __UQMID_CTRL_H

struct qmi_dev;
struct qmi_service;

int uqmi_ctrl_request_clientid(struct qmi_service *service);
int uqmi_ctrl_release_clientid(struct qmi_service *service);
struct qmi_service *uqmi_ctrl_generate(struct qmi_dev *qmi);

#endif /* __UQMID_CTRL_H */
